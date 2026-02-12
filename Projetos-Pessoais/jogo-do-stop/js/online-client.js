window.socket = io("https://jogo-do-stop-backend.onrender.com", {
  transports: ["websocket", "polling"]
});

/*window.socket = io("http://localhost:3000", {
  transports: ["websocket", "polling"]
});*/

window.api = {
  createRoom: function ({ nickname, avatar, password, config }, cb) {
    window.socket.emit("room:create", { nickname, avatar, password, config }, function (res) {
      if (res && res.ok && res.roomId && res.hostKey) {
        localStorage.setItem("hostKey_" + res.roomId, res.hostKey);
      }
      if (cb) cb(res);
    });
  },

  joinRoom: function ({ roomId, nickname, avatar, password }, cb) {
    window.socket.emit("room:join", { roomId, nickname, avatar, password }, cb);
  },

  startRound: function (roomId, cb) {
    const hostKey = localStorage.getItem("hostKey_" + roomId) || null;
    window.socket.emit("game:startRound", { roomId: roomId, hostKey: hostKey }, cb);
  },

  submit: function (roomId, answers, score, cb) {
    window.socket.emit("game:submit", { roomId, answers, score }, cb);
  },

  stopRound: function (roomId, cb) {
    window.socket.emit("game:stopRound", { roomId: roomId }, cb);
  },

  nextReady: function (roomId, cb) {
    window.socket.emit("game:nextReady", { roomId: roomId }, cb);
  }

};