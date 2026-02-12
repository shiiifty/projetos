import express from "express";
import http from "http";
import { Server } from "socket.io";
import path from "path";
import { fileURLToPath } from "url";
import { config } from "process";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const server = http.createServer(app);
const io = new Server(server, {
  cors: {
    origin: [
      "http://localhost:5500",
      "http://127.0.0.1:5500",
      "https://shiiifty.github.io",
      "https://jogo-do-stop-backend.onrender.com" 
    ],
    methods: ["GET", "POST"]
  }
});


app.use(express.static(path.join(__dirname, "..")));

const rooms = new Map();
const roomDeleteTimers = new Map();

function safeState(state) {
  return {
    round: state.round,
    running: state.running,
    letter: state.letter,
    roundStartAt: state.roundStartAt,
    gameStartAt: state.gameStartAt,
    acceptingSubmits: state.acceptingSubmits,
    endRequested: state.endRequested,
    nextRoundStartAt: state.nextRoundStartAt,
    ready: state.readySet ? state.readySet.size : 0
  };
}

function cancelRoomDeletion(roomId) {
  const t = roomDeleteTimers.get(roomId);
  if (t) {
    clearTimeout(t);
    roomDeleteTimers.delete(roomId);
  }
}

function scheduleRoomDeletion(roomId, ms = 1000000) {
  cancelRoomDeletion(roomId);
  const t = setTimeout(() => {
    rooms.delete(roomId);
    roomDeleteTimers.delete(roomId);
  }, ms);
  roomDeleteTimers.set(roomId, t);
}


function makeRoomId() {
  return Math.random().toString(36).slice(2, 8).toUpperCase();
}

function publicRoom(roomId) {
  const room = rooms.get(roomId);
  if (!room) return null;

  return {
    roomId,
    hostId: room.hostId,
    config: room.config,
    state: safeState(room.state),
    players: [...room.players.entries()].map(([socketId, p]) => ({
      socketId,
      nickname: p.nickname,
      avatar: p.avatar,
      score: p.score
    }))
  };

}

function publicRoomForSocket(roomId, socketId) {
  const room = rooms.get(roomId);
  if (!room) return null;

  const base = publicRoom(roomId);
  if (!base) return null;

  base.password = room.hostId === socketId ? (room.password || null) : null;
  return base;
}

function emitRoomUpdate(roomId) {
  const room = rooms.get(roomId);
  if (!room) return;

  for (const sid of room.players.keys()) {
    const payload = publicRoomForSocket(roomId, sid);
    if (payload) io.to(sid).emit("room:update", payload);
  }
}

function finalizeRound(roomId) {
  const room = rooms.get(roomId);
  if (!room) return;

  if (!room.state.acceptingSubmits) return;

  room.state.acceptingSubmits = false;
  room.state.running = false;
  room.state.endRequested = false;

  for (const p of room.players.values()) {
    if (p.lastSubmit && p.lastSubmit.score != null) {
      p.score += p.lastSubmit.score;
    }
  }

  io.to(roomId).emit("game:roundEnded", {
    round: room.state.round,
    results: [...room.players.values()].map((p) => ({
      nickname: p.nickname,
      scoreTotal: p.score,
      lastScore: (p.lastSubmit && p.lastSubmit.score != null) ? p.lastSubmit.score : 0,
      answers: (p.lastSubmit && p.lastSubmit.answers != null) ? p.lastSubmit.answers : null
    }))
  });

  for (const p of room.players.values()) p.lastSubmit = null;

  room.state.running = false;
  room.state.readySet = new Set();
  room.state.nextRoundStartAt = null;
  if (room.state.nextRoundStartTimer) {
    clearTimeout(room.state.nextRoundStartTimer);
    room.state.nextRoundStartTimer = null;
  }


  emitRoomUpdate(roomId);
}

function startRoundNow(roomId) {
  const room = rooms.get(roomId);
  if (!room) return;

  if (room.state.running) return;
  if (room.state.round >= room.config.rounds) return;

  room.state.acceptingSubmits = true;
  room.state.endRequested = false;
  if (room.state.endTimer) {
    clearTimeout(room.state.endTimer);
    room.state.endTimer = null;
  }

  room.state.round += 1;
  room.state.running = true;

  let pool = room.config.letters.split("");
  const alphabet_s = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  let alphabet = alphabet_s.split("");

  pool = (pool.length > 0) ? pool : alphabet;

  room.state.letter = pool[Math.floor(Math.random() * pool.length)];
  room.state.roundStartAt = Date.now();

  room.state.readySet = new Set();
  room.state.nextRoundStartAt = null;
  if (room.state.nextRoundStartTimer) {
    clearTimeout(room.state.nextRoundStartTimer);
    room.state.nextRoundStartTimer = null;
  }

  io.to(roomId).emit("game:roundStarted", {
    round: room.state.round,
    rounds: room.config.rounds,
    letter: room.state.letter,
    timePerRound: room.config.timePerRound,
    roundStartAt: room.state.roundStartAt
  });

  emitRoomUpdate(roomId);
}

function emitPublicRoomsUpdate() {
  const list = [];

  for (const [roomId, room] of rooms.entries()) {
    if (!room) continue;
    if (room.password) continue;

    const playerCount =
      room.players && room.players.size ? room.players.size : 0;

    if (playerCount === 0) continue;

    const hostPlayer =
      room.players && room.hostId ? room.players.get(room.hostId) : null;

    const hostNickname =
      hostPlayer && hostPlayer.nickname ? hostPlayer.nickname : "—";

    list.push({
      roomId: roomId,
      players: playerCount,
      host: hostNickname,
      running: room.state && room.state.running ? true : false,
      rounds: room.config && typeof room.config.rounds !== "undefined" ? room.config.rounds : null,
      timePerRound:
        room.config && typeof room.config.timePerRound !== "undefined"
          ? room.config.timePerRound
          : null,
      createdAt: typeof room.createdAt !== "undefined" ? room.createdAt : null
    });
  }

  list.sort((a, b) => ((b.createdAt || 0) - (a.createdAt || 0)));
  io.emit("rooms:publicUpdate", { rooms: list });
}


io.on("connection", (socket) => {
  socket.on("room:listPublic", (cb) => {
    try {
      const list = [];

      for (const [roomId, room] of rooms.entries()) {
        if (!room) continue;

        if (room.password) continue;

        const playerCount =
          room.players && room.players.size ? room.players.size : 0;

        if (playerCount === 0) continue;

        const hostPlayer =
          room.players && room.hostId
            ? room.players.get(room.hostId)
            : null;

        const hostNickname =
          hostPlayer && hostPlayer.nickname
            ? hostPlayer.nickname
            : "—";

        list.push({
          roomId: roomId,
          players: playerCount,
          host: hostNickname,
          running: room.state && room.state.running ? true : false,
          rounds: room.config && room.config.rounds ? room.config.rounds : null,
          timePerRound:
            room.config && room.config.timePerRound
              ? room.config.timePerRound
              : null,
          createdAt: room.createdAt ? room.createdAt : null
        });
      }

      list.sort((a, b) => (b.createdAt || 0) - (a.createdAt || 0));

      if (cb) cb({ ok: true, rooms: list });
    } catch (err) {
      if (cb) cb({ ok: false, error: "Erro ao listar salas." });
    }
  });

  socket.on("game:getState", ({ roomId }, cb) => {
    roomId = String(roomId || "").trim().toUpperCase();
    const room = rooms.get(roomId);

    if (!room) {
      if (cb) cb({ ok: false, error: "Sala não existe." });
      return;
    }

    if (cb) cb({
      ok: true,
      state: safeState(room.state),
      config: room.config
    });

  });

  socket.on("game:nextReady", ({ roomId }, cb) => {
    roomId = String(roomId || "").trim().toUpperCase();
    const room = rooms.get(roomId);

    let requestResult = false;

    if (!room) { if (cb) cb({ ok: false, error: "Sala não existe." }); return; }

    if (room.state.running) { if (cb) cb({ ok: false, error: "A ronda ainda está a decorrer." }); return; }

    if (room.state.round >= room.config.rounds) {
      requestResult = true;
      socket.emit("game:showResults", {
        results: [...room.players.values()].map((p) => ({
          nickname: p.nickname,
          scoreTotal: p.score,
        }))
      });
    }

    if (!room.state.readySet) room.state.readySet = new Set();
    room.state.readySet.add(socket.id);

    if (!requestResult) {
      io.to(roomId).emit("game:nextReadyUpdate", {
        ready: room.state.readySet.size,
        total: room.players.size
      });
    }

    if (room.state.readySet.size === room.players.size) {
      if (room.state.nextRoundStartTimer) return; 

      const startAt = Date.now() + 3000;
      room.state.nextRoundStartAt = startAt;

      io.to(roomId).emit("game:nextRoundStarting", { startAt: startAt });

      room.state.nextRoundStartTimer = setTimeout(() => {
        const r = rooms.get(roomId);
        if (!r) return;
        r.state.nextRoundStartTimer = null;
        startRoundNow(roomId);
      }, 3000);
    }

    if (cb) cb({ ok: true });
  });

  socket.on("room:create", ({ nickname, avatar, password, config }, cb) => {
    const roomId = makeRoomId();
    cancelRoomDeletion(roomId);

    const safeConfig = {
      timePerRound: Number(config && config.timePerRound),
      rounds: Number(config && config.rounds),
      letters: String(config && config.letters || "").toUpperCase().replace(/[^A-Z]/g, "")
    };

    rooms.set(roomId, {
      createdAt: Date.now(),
      hostId: socket.id,
      password: password || null,
      config: safeConfig,
      players: new Map(),
      state: {
        round: 0,
        running: false,
        letter: null,
        roundStartAt: null,
        gameStartAt: null,
        acceptingSubmits: false,
        endRequested: false,
        endTimer: null,
        readySet: new Set(),
        nextRoundStartTimer: null,
        nextRoundStartAt: null
      }
    });

    const room = rooms.get(roomId);
    room.players.set(socket.id, { nickname, avatar, score: 0 });

    socket.join(roomId);

    if (cb) cb({ ok: true, roomId });

    emitRoomUpdate(roomId);
    emitPublicRoomsUpdate();
  });

  socket.on("game:goToGame", ({ roomId, seconds }, cb) => {
    roomId = String(roomId || "").trim().toUpperCase();
    const room = rooms.get(roomId);

    if (!room) {
      if (cb) cb({ ok: false, error: "Sala não existe." });
      return;
    }

    if (socket.id !== room.hostId) {
      if (cb) cb({ ok: false, error: "Só o host pode iniciar." });
      return;
    }

    if (room.state.gameStartAt && room.state.gameStartAt > Date.now()) {
      if (cb) cb({ ok: false, error: "O jogo já está a iniciar." });
      return;
    }

    const countdown = Number(seconds) || 5;
    const startAt = Date.now() + countdown * 1000;
    room.state.gameStartAt = startAt;

    io.to(roomId).emit("game:starting", { roomId, startAt });

    const ROUND_DELAY_MS = 200;
    const msUntilRound = Math.max(0, (startAt + ROUND_DELAY_MS) - Date.now());

    setTimeout(() => {

      const r = rooms.get(roomId);
      if (!r) return;

      if (r.state.running) return;

      r.state.round = 1;
      r.state.running = true;
      r.state.acceptingSubmits = true;
      r.state.endRequested = false;
      if (r.state.endTimer) { clearTimeout(r.state.endTimer); r.state.endTimer = null; }


      let pool = r.config.letters.split("");
      const alphabet_s = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      let alphabet = alphabet_s.split("");

      pool = (pool.length > 0) ? pool : alphabet;

      r.state.letter = pool[Math.floor(Math.random() * pool.length)];
      r.state.roundStartAt = Date.now();

      io.to(roomId).emit("game:roundStarted", {
        round: r.state.round,
        rounds: r.config.rounds,
        letter: r.state.letter,
        timePerRound: r.config.timePerRound,
        roundStartAt: r.state.roundStartAt
      });

      emitRoomUpdate(roomId);
    }, msUntilRound);


    if (cb) cb({ ok: true });
  });


  socket.on("room:join", ({ roomId, nickname, avatar, password }, cb) => {
    const norm = String(roomId || "").trim().toUpperCase();

    roomId = String(roomId || "").trim().toUpperCase();
    const room = rooms.get(roomId);
    cancelRoomDeletion(roomId);


    if (!room) {
      if (cb) cb({ ok: false, error: "Sala não existe." });
      return;
    }


    if (room.password) {
      if (!password || !String(password).trim()) {
        if (cb) cb({ ok: false, error: "Password necessária." });
        return;
      }

      if (String(password) !== String(room.password)) {
        if (cb) cb({ ok: false, error: "Password errada." });
        return;
      }
    }


    room.players.set(socket.id, { nickname, avatar, score: 0 });
    socket.join(roomId);

    if (cb) cb({ ok: true });

    if (!room.hostId || !room.players.has(room.hostId)) {
      room.hostId = socket.id;
    } 

    emitRoomUpdate(roomId);
    emitPublicRoomsUpdate();
  });

  socket.on("game:startRound", ({ roomId }, cb) => {
    roomId = String(roomId || "").trim().toUpperCase();
    const room = rooms.get(roomId);

    if (!room) {
      if (cb) cb({ ok: false });
      return;
    }

    if (socket.id !== room.hostId) {
      if (cb) cb({ ok: false, error: "Só o host." });
      return;
    }

    if (room.state.running) {
      if (cb) cb({ ok: false, error: "Round já a correr." });
      return;
    }

    room.state.round += 1;
    room.state.running = true;
    room.state.acceptingSubmits = true;
    room.state.endRequested = false;
    if (room.state.endTimer) { clearTimeout(room.state.endTimer); room.state.endTimer = null; }

    let pool = room.config.letters.split("");
    const alphabet_s = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    let alphabet = alphabet_s.split("");

    pool = (pool.length > 0) ? pool : alphabet;

    room.state.letter = pool[Math.floor(Math.random() * pool.length)];
    room.state.roundStartAt = Date.now();

    io.to(roomId).emit("game:roundStarted", {
      round: room.state.round,
      rounds: room.config.rounds,
      letter: room.state.letter,
      timePerRound: room.config.timePerRound,
      roundStartAt: room.state.roundStartAt
    });

    if (cb) cb({ ok: true });
  });

  socket.on("game:submit", ({ roomId, answers, score }, cb) => {
    roomId = String(roomId || "").trim().toUpperCase();
    const room = rooms.get(roomId);

    if (!room) { if (cb) cb({ ok: false }); return; }
    if (!room.state.acceptingSubmits) { if (cb) cb({ ok: false, error: "Submissões fechadas." }); return; }

    const player = room.players.get(socket.id);
    if (player) player.lastSubmit = { answers, score };

    if (cb) cb({ ok: true });

    const allSubmitted = [...room.players.values()].every((p) => p.lastSubmit && p.lastSubmit.score != null);
    if (allSubmitted) {
      if (room.state.endTimer) { clearTimeout(room.state.endTimer); room.state.endTimer = null; }
      finalizeRound(roomId);
    }
  });


  socket.on("game:stopRound", ({ roomId }, cb) => {
    roomId = String(roomId || "").trim().toUpperCase();
    const room = rooms.get(roomId);

    if (!room) { if (cb) cb({ ok: false, error: "Sala não existe." }); return; }
    if (!room.state.acceptingSubmits) { if (cb) cb({ ok: false, error: "Round já terminou." }); return; }

    io.to(roomId).emit("game:forceEndRound", { stoppedBy: socket.id });

    if (!room.state.endRequested) {
      room.state.endRequested = true;
      room.state.running = false;

      if (room.state.endTimer) clearTimeout(room.state.endTimer);
      room.state.endTimer = setTimeout(() => {
        room.state.endTimer = null;
        finalizeRound(roomId);
      }, 1200);
    }

    if (cb) cb({ ok: true });
  });



  socket.on("disconnect", (reason) => {
    for (const [roomId, room] of rooms.entries()) {
      if (!room.players.has(socket.id)) continue;

      room.players.delete(socket.id);

      if (room.hostId === socket.id) {
        const first = room.players.keys().next().value;
        room.hostId = first || null;
      }

      if (room.players.size === 0) {
        room.state.gameStartAt = null;
        scheduleRoomDeletion(roomId, 30000);
      } else {
        emitRoomUpdate(roomId);
        emitPublicRoomsUpdate();
      }
    }

  });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, "0.0.0.0", () => console.log("Server on http://localhost:" + PORT));
