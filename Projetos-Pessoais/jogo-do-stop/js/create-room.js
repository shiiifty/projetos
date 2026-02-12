document.addEventListener("DOMContentLoaded", function () {
  const publicBtn = document.getElementById("public-btn");
  const privateBtn = document.getElementById("private-btn");

  const popup = document.getElementById("popup-overlay");
  const closePopup = document.getElementById("close-popup");
  const confirmPopup = document.getElementById("confirm-popup");
  const wrongPwMsg = document.getElementById("wrong-pw-msg");
  const senhaInput = document.getElementById("senha-input");

  const configOverlay   = document.getElementById("config-overlay");
  const configTime      = document.getElementById("config-time");
  const configLetters   = document.getElementById("config-letters");
  const configRounds    = document.getElementById("config-rounds");
  const configCancelBtn = document.getElementById("config-cancel");
  const configStartBtn  = document.getElementById("config-start");

  const ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  

  function getPlayerInfo() {
    const nickname = localStorage.getItem("playerNickname") || "Player";
    const avatar = localStorage.getItem("playerAvatar") || "default";
 
    const gameConfig = JSON.parse(localStorage.getItem("gameConfig") || "null");

    const config = gameConfig;
    return { nickname, avatar, config };
  }

  function goToLobby(roomId) {
    localStorage.setItem("lastRoomId", roomId);

    window.location.href = "./lobby.html?room=" + encodeURIComponent(roomId);
  }

  function createRoom(passwordOrNull) {
    const info = getPlayerInfo();

    window.api.createRoom(
      {
        nickname: info.nickname,
        avatar: info.avatar,
        password: passwordOrNull, 
        config: info.config
      },
      function (res) {
        if (!res || !res.ok) {
          alert(res && res.error ? res.error : "Erro a criar sala.");
          return;
        }

        goToLobby(res.roomId);
      }
    );
  }

  if (configCancelBtn && configOverlay) {
    configCancelBtn.addEventListener("click", () => {
      configOverlay.classList.add("hidden");
    });
  }

  if (publicBtn) {
    publicBtn.addEventListener("click", function () {
      localStorage.removeItem("roomPassword");

      configOverlay.classList.remove("hidden");

      if (configStartBtn) {
        configStartBtn.addEventListener("click", () => {
          const time = parseInt(configTime.value, 10) || 60;
          
          let excluded = (configLetters.value || "")
          .toUpperCase()
          .replace(/[^A-Z]/g, "");

          const letters = ALPHABET
          .split("")
          .filter(l => !excluded.includes(l))
          .join("");

          const total_rounds = parseInt(configRounds.value, 10) || 10;

          const gameConfig = {
            timePerRound: time,
            letters: letters,
            rounds: total_rounds,
          };

          localStorage.setItem("gameConfig", JSON.stringify(gameConfig));
          createRoom(null);
        });
      }
    });
  }

  if (privateBtn && popup) {
    privateBtn.addEventListener("click", function () {
      configOverlay.classList.remove("hidden");

      if (configStartBtn) {
        configStartBtn.addEventListener("click", () => {
          const time = parseInt(configTime.value, 10) || 60;
          
          let excluded = (configLetters.value || "")
          .toUpperCase()
          .replace(/[^A-Z]/g, "");

          const letters = ALPHABET
          .split("")
          .filter(l => !excluded.includes(l))
          .join("");

          const total_rounds = parseInt(configRounds.value, 10) || 10;

          const gameConfig = {
            timePerRound: time,
            letters: letters,
            rounds: total_rounds,
          };

          localStorage.setItem("gameConfig", JSON.stringify(gameConfig));
          popup.classList.add("show");
          if (wrongPwMsg) wrongPwMsg.classList.remove("show");
          if (senhaInput) senhaInput.value = "";
        });
      }
    });
  }

  if (closePopup && popup) {
    closePopup.addEventListener("click", function () {
      popup.classList.remove("show");
    });
  }

  if (confirmPopup && senhaInput && popup) {
    confirmPopup.addEventListener("click", function () {
      const pw = senhaInput.value.trim();

      if (pw.length !== 4 || isNaN(pw)) {
        if (wrongPwMsg) wrongPwMsg.classList.add("show");
        return;
      }

      if (wrongPwMsg) wrongPwMsg.classList.remove("show");
      popup.classList.remove("show");

      localStorage.setItem("roomPassword", pw);

      createRoom(pw);
    });
  }

  if (popup) {
    popup.addEventListener("click", function (e) {
      if (e.target === popup) popup.classList.remove("show");
    });
  }
});
