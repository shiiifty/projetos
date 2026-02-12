document.addEventListener("DOMContentLoaded", function () {
  const params = new URLSearchParams(window.location.search);
  const roomId = String(params.get("room") || "").trim().toUpperCase();

  const modal = document.getElementById("start-modal");
  const countdownEl = document.getElementById("countdown");
  let countdownTimer = null;
  
  function openCountdownModal(startAt) {
    if (!modal || !countdownEl) return;

    modal.classList.remove("hidden");

    function tick() {
      const msLeft = startAt - Date.now();
      const secLeft = Math.max(0, Math.ceil(msLeft / 1000));
      countdownEl.textContent = secLeft;

      if (msLeft <= 0) {
        clearInterval(countdownTimer);
        countdownTimer = null;

        modal.classList.add("hidden");
        window.location.href = "/html/gameOnline.html?room=" + encodeURIComponent(roomId);
      }
    }

    if (countdownTimer) clearInterval(countdownTimer);
    tick();
    countdownTimer = setInterval(tick, 200);
  }

  if (!roomId) {
    alert("Falta o código da sala no URL. Ex: lobby.html?room=ABC123");
    return;
  }

  const roomCodeEl = document.getElementById("room-code");
  const playersEl = document.getElementById("players-list");
  const startBtn = document.getElementById("start-game-btn");
  const hostMsg = document.getElementById("host-info");
  const pwLine = document.getElementById("room-password-line");
  const pwEl = document.getElementById("room-password");

  if (roomCodeEl) roomCodeEl.textContent = roomId;

  const nickname = localStorage.getItem("playerNickname") || "Player";
  const avatar = localStorage.getItem("playerAvatar") || "default";
  const savedPassword = localStorage.getItem("roomPassword") || "";

  window.api.joinRoom({ roomId, nickname, avatar, password: savedPassword }, function (res) {
    if (res && res.ok) return;

    alert((res && res.error) ? res.error : "Erro ao entrar na sala.");
    window.location.href = "/html/play.html";
  });

  window.socket.on("room:update", function (room) {
    if (!room) return;
    if (String(room.roomId).toUpperCase() !== roomId) return;

    if (playersEl) {
      playersEl.innerHTML = "";
      (room.players || []).forEach(function (p) {
        const li = document.createElement("li");

        li.style.display = "flex";
        li.style.flexDirection = "row";
        li.style.alignItems = "center";
        li.style.gap = "14px";
        li.style.padding = "12px 20px";

        const img = document.createElement("img");

        img.src = p.avatar;
        img.alt = p.nickname + " avatar";
        
        
        img.style.width = "56px";
        img.style.height = "56px";
        img.style.borderRadius = "50%";
        img.style.objectFit = "cover";
        img.style.flexShrink = "0";

        
        const name = document.createElement("span");
        name.textContent = p.nickname;

        name.style.fontSize = "22px";
        name.style.lineHeight = "1";
        
        li.appendChild(img)
        li.appendChild(name);
        playersEl.appendChild(li);
      });
    }

    const myId = window.socket.id;
    const isHost = !!myId && room.hostId === myId;

    if (pwLine && pwEl) {
      if (room.password) {
        pwLine.classList.remove("hidden");
        pwEl.classList.remove("hidden");
        pwEl.textContent = room.password;
      } else {
        pwLine.classList.add("hidden");
        pwEl.classList.add("hidden");
        pwEl.textContent = "";
      }
    }

    if (startBtn) startBtn.style.display = isHost ? "inline-block" : "none";
    if (hostMsg) hostMsg.textContent = isHost
      ? "És o host. Podes iniciar o jogo."
      : "À espera que o host inicie o jogo...";
  });

   if (startBtn) {
    startBtn.addEventListener("click", function () {
      window.socket.emit("game:goToGame", { roomId, seconds: 5 }, function (res) {
        if (res && res.ok) return;
        alert((res && res.error) ? res.error : "Erro ao iniciar o jogo.");
      });
    });
  }

  window.socket.on("game:starting", function (payload) {
    if (!payload || String(payload.roomId).toUpperCase() !== roomId) return;
    if (!payload.startAt) return;
    openCountdownModal(payload.startAt);
  });
});
