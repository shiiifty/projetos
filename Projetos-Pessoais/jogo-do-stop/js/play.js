document.addEventListener("DOMContentLoaded", function () {
  const params = new URLSearchParams(window.location.search);
  const roomIdRaw = params.get("room");
  const roomId = roomIdRaw ? roomIdRaw.toUpperCase() : null;
  const isOnline = !!roomId;

  const nicknameSpan = document.getElementById("nickname-span");
  const letterSpan   = document.getElementById("letter-span");
  const timerSpan    = document.getElementById("timer-span");
  const roundSpan    = document.getElementById("round-span");
  const startBtn     = document.getElementById("start-btn");
  const stopBtn      = document.getElementById("stop-btn");
  const answerInputs = document.querySelectorAll(".answer-input");

  const scoreOverlay   = document.getElementById("score-overlay");
  const scoreValueSpan = document.getElementById("score-value");
  const validCountSpan = document.getElementById("valid-count");
  const totalCountSpan = document.getElementById("total-count");
  const totalScoreSpan = document.getElementById("total-score");
  const closeScoreBtn  = document.getElementById("close-score-btn");

  const compareOverlay = document.getElementById("compare-overlay");
  const compareTitle = document.getElementById("compare-title");
  const compareSubtitle = document.getElementById("compare-subtitle");
  const compareCategoryTitle = document.getElementById("compare-category-title");
  const compareTable = document.getElementById("compare-table");

  const comparePrevBtn = document.getElementById("compare-prev");
  const compareNextBtn = document.getElementById("compare-next");
  const comparePauseBtn = document.getElementById("compare-pause");
  const compareCountdownSpan = document.getElementById("compare-countdown");
  const compareNextRoundBtn = document.getElementById("compare-next-round");
  const compareReadyText = document.getElementById("compare-ready");
  const stopWarning = document.getElementById("incorrect-stop");

  const resultsOverlay = document.getElementById("results-overlay");
  const resultsTable = document.getElementById("results-table");
  const closeBtn = document.getElementById("close-btn");
  const playAgainBtn = document.getElementById("play-again-btn");

  let readySent = false;
  let isCountingNextRound = false;
  
  let nextRoundCountdownTimer = null;

  if (closeBtn) {
    closeBtn.addEventListener("click", function () {
      window.location.href = "../index.html";
    });
  }

  if (playAgainBtn) {
    playAgainBtn.addEventListener("click", function () {
      if (window.history.length > 1) window.history.back();
      else window.location.href = "../index.html";
    });
  }

  if (compareNextRoundBtn) {
    compareNextRoundBtn.addEventListener("click", function () {
      if (!isOnline || !window.api || !window.api.nextReady) return;
      if (readySent) return;

      readySent = true;
      compareNextRoundBtn.disabled = true;
      compareNextRoundBtn.textContent = "A aguardar jogadores...";

      window.api.nextReady(roomId, function (res) {
        if (!res || !res.ok) {
          readySent = false;
          compareNextRoundBtn.disabled = false;
          compareNextRoundBtn.textContent = "Próxima ronda";
          alert((res && res.error) ? res.error : "Erro ao marcar pronto.");
        }
      });
    });
  }

  const compareState = {
    open: false,
    paused: false,
    stepSeconds: 5,
    secondsLeft: 5,
    timerTick: null,
    categories: [],
    players: [],
    index: 0,
    round: 0
  };

  function showStopWarning(show) {
    if (!stopWarning) return;
    if (show) stopWarning.classList.remove("hidden");
    else stopWarning.classList.add("hidden");
  }


  function openCompareOverlay(payload) {
    if (!compareOverlay) return;

    compareState.round = payload.round || 0;

    var results = payload.results || [];
    compareState.players = results.map(function (p) {
      return {
        nickname: p.nickname,
        lastScore: (p.lastScore != null) ? p.lastScore : 0,
        scoreTotal: (p.scoreTotal != null) ? p.scoreTotal : 0,
        answers: p.answers || {}
      };
    });

    var catSet = {};
    compareState.players.forEach(function (p) {
      var a = p.answers || {};
      Object.keys(a).forEach(function (k) { catSet[k] = true; });
    });
    compareState.categories = Object.keys(catSet);

    if (compareState.categories.length === 0 && answerInputs && answerInputs.length) {
      compareState.categories = Array.from(answerInputs).map(function (inp) {
        return inp.dataset.category;
      }).filter(Boolean);
    }

    compareState.index = 0;
    compareState.paused = false;
    compareState.secondsLeft = compareState.stepSeconds;

    compareState.open = true;
    compareOverlay.classList.remove("hidden");

    readySent = false;
    if (compareNextRoundBtn) {
      compareNextRoundBtn.disabled = false;
      compareNextRoundBtn.textContent = (compareState.round >= total_rounds) ? "Ver resultados" : "Próxima ronda";
    }
    if (compareReadyText) compareReadyText.textContent = "";


    renderCompareStep();
    startCompareTimer();
  }

  function renderResultsTable(payload) {
    var html = "";

    html += "<div class='compare-row'>";
    html += "<div class='compare-cell compare-head'>Jogador</div>";
    html += "<div class='compare-cell compare-head'>Pontos</div>";
    html += "</div>";

    compareState.players.forEach(function (p, _) {
      html += "<div class='compare-row'>";
      html += "<div class='compare-cell'><strong>" + escapeHtml(p.nickname) + "</strong></div>";
      html += "<div class='compare-cell'><span class='badge'>" + escapeHtml(p.scoreTotal) + "</span></div>";
      html += "</div>";
    });

    resultsTable.innerHTML = html;
  }

  function show_results(payload) {
    if (!resultsOverlay) return;
    renderResultsTable(payload);
    if (compareOverlay) compareOverlay.classList.add("hidden");
    resultsOverlay.classList.remove("hidden");
  }

  function normalizeAnswer(s) {
    return String(s || "").trim().toLowerCase();
  }

  function computeCategoryMarks(category) {
    var vals = compareState.players.map(function (p) {
      var v = (p.answers && p.answers[category] != null) ? String(p.answers[category]) : "";
      return v.trim();
    });

    var counts = {};
    vals.forEach(function (v) {
      if (!v) return;

      var ok = false;
      if (typeof isValidWordForCategoryAndLetter === "function") {
        ok = isValidWordForCategoryAndLetter(category, v, currentLetter);
      } else {
        ok = currentLetter ? (v.toUpperCase().indexOf(currentLetter) === 0) : true;
      }
      if (!ok) return;

      var n = normalizeAnswer(v);
      if (!n) return;
      counts[n] = (counts[n] || 0) + 1;
    });

    return vals.map(function (v) {
      if (!v) return { status: "bad", points: 0 };

      var ok = false;
      if (typeof isValidWordForCategoryAndLetter === "function") {
        ok = isValidWordForCategoryAndLetter(category, v, currentLetter);
      } else {
        ok = currentLetter ? (v.toUpperCase().indexOf(currentLetter) === 0) : true;
      }

      if (!ok) return { status: "bad", points: 0 };

      var n = normalizeAnswer(v);

      if (n && counts[n] > 1) return { status: "dup", points: 10 };

      return { status: "ok", points: 20 };
    });
  }


  function renderCompareStep() {
    if (!compareState.open) return;

    var cats = compareState.categories;
    if (!cats || cats.length === 0) {
      if (compareCategoryTitle) compareCategoryTitle.textContent = "Sem categorias";
      if (compareTable) compareTable.innerHTML = "<div class='muted'>Não há respostas para comparar.</div>";
      return;
    }

    var idx = compareState.index;
    if (idx < 0) idx = 0;
    if (idx >= cats.length) idx = cats.length - 1;
    compareState.index = idx;

    var category = cats[idx];

    if (compareTitle) compareTitle.textContent = "Comparação da ronda";
    if (compareSubtitle) compareSubtitle.textContent =
      "Ronda " + compareState.round + " • " + (idx + 1) + "/" + cats.length;

    if (compareCategoryTitle) compareCategoryTitle.textContent = String(category);

    var marks = computeCategoryMarks(category);

    if (comparePrevBtn) comparePrevBtn.disabled = (idx === 0);
    if (compareNextBtn) compareNextBtn.disabled = (idx === cats.length - 1);
    if (comparePauseBtn) comparePauseBtn.textContent = compareState.paused ? "▶ Continuar" : "⏸ Pause";
    if (compareCountdownSpan) compareCountdownSpan.textContent = String(compareState.secondsLeft);

    if (!compareTable) return;

    var html = "";

    html += "<div class='compare-row'>";
    html += "<div class='compare-cell compare-head'>Jogador</div>";
    html += "<div class='compare-cell compare-head'>Resposta</div>";
    html += "<div class='compare-cell compare-head'>Pontos</div>";
    html += "</div>";

    compareState.players.forEach(function (p, i) {
      var ans = (p.answers && p.answers[category] != null) ? String(p.answers[category]).trim() : "";
      var m = marks[i];

      var badgeText = "";
      if (m.status === "ok") badgeText = "✅ +20";
      else if (m.status === "dup") badgeText = "🟡 +10";
      else badgeText = "❌ 0";

      html += "<div class='compare-row'>";
      html += "<div class='compare-cell'><strong>" + escapeHtml(p.nickname) + "</strong></div>";
      html += "<div class='compare-cell'>" + (ans ? escapeHtml(ans) : "<span class='answer-empty'>(vazio)</span>") + "</div>";
      html += "<div class='compare-cell'><span class='badge'>" + badgeText + "</span></div>";
      html += "</div>";
    });

    compareTable.innerHTML = html;
  }

  function escapeHtml(s) {
    return String(s)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#039;");
  }

  function stopCompareTimer() {
    if (compareState.timerTick) {
      clearInterval(compareState.timerTick);
      compareState.timerTick = null;
    }
  }

  function startCompareTimer() {
    stopCompareTimer();

    compareState.timerTick = setInterval(function () {
      if (!compareState.open) return;
      if (compareState.paused) return;

      compareState.secondsLeft -= 1;
      if (compareState.secondsLeft <= 0) {
        compareState.secondsLeft = compareState.stepSeconds;

        if (compareState.index < compareState.categories.length - 1) {
          compareState.index += 1;
          renderCompareStep();
        } else {
          compareState.paused = true;
          renderCompareStep();
        }
      } else {
        if (compareCountdownSpan) compareCountdownSpan.textContent = String(compareState.secondsLeft);
      }
    }, 1000);
  }


  if (comparePrevBtn) comparePrevBtn.addEventListener("click", function () {
    if (!compareState.open) return;
    if (compareState.index > 0) compareState.index -= 1;
    compareState.secondsLeft = compareState.stepSeconds;
    renderCompareStep();
  });

  if (compareNextBtn) compareNextBtn.addEventListener("click", function () {
    if (!compareState.open) return;
    if (compareState.index < compareState.categories.length - 1) compareState.index += 1;
    compareState.secondsLeft = compareState.stepSeconds;
    renderCompareStep();
  });

  if (comparePauseBtn) comparePauseBtn.addEventListener("click", function () {
    if (!compareState.open) return;
    compareState.paused = !compareState.paused;
    renderCompareStep();
  });

  let currentLetter = null;
  let total_rounds = 10;
  let currentRound = 0;
  let totalScore = 0;

  const nickname = localStorage.getItem("playerNickname") || "Player 1";
  const avatar = localStorage.getItem("playerAvatar") || "default";
  if (nicknameSpan) nicknameSpan.textContent = nickname;

  const defaultConfig = {
    timePerRound: 60,
    letters: "",
    rounds: 10
  };

  let config = { ...defaultConfig };
  const saved = localStorage.getItem("gameConfig");

  if (saved) {
    try {
      const parsed = JSON.parse(saved);
      config = { ...defaultConfig, ...parsed };
    } catch (e) {
      console.warn("Config inválida, uso defaults.");
    }
  }

  let timeLeft = config.timePerRound;
  total_rounds = config.rounds;

  let timerId = null;
  let roundRunning = false;
  let stopRequested = false;


  if (roundSpan) roundSpan.textContent = "0/" + total_rounds;

  function setInputsEnabled(enabled) {
    answerInputs.forEach(function (input) {
      input.disabled = !enabled;
      if (!enabled) input.blur();
    });
  }

  function resetRoundUI() {
    timeLeft = config.timePerRound;
    if (timerSpan) timerSpan.textContent = String(timeLeft);
    if (letterSpan) letterSpan.textContent = "-";

    answerInputs.forEach(function (input) {
      input.classList.remove("valida", "erro");
      input.value = "";
    });

    setInputsEnabled(false);
    updateStopButtonState();
  }

  function generateRandomLetter() {
    const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const excluded = new Set(
      String(config.letters || "")
        .toUpperCase()
        .replace(/[^A-Z]/g, "")
        .split("")
    );

    let pool = []

    for (const ch of excluded) {
      pool.push(ch);
    }

    if (pool.length === 0) pool = alphabet.split("");

    const idx = Math.floor(Math.random() * pool.length);
    return pool[idx];
  }

  function startRoundOffline() {
    if (roundRunning) return;

    if (currentRound >= total_rounds) {
      alert("Já jogaste todas as rondas deste jogo!");
      return;
    }

    currentRound++;
    if (roundSpan) roundSpan.textContent = currentRound + "/" + total_rounds;

    roundRunning = true;

    currentLetter = generateRandomLetter();
    if (letterSpan) letterSpan.textContent = currentLetter;

    setInputsEnabled(true);

    timeLeft = config.timePerRound;
    if (timerSpan) timerSpan.textContent = String(timeLeft);

    timerId = setInterval(function () {
      timeLeft--;
      if (timerSpan) timerSpan.textContent = String(timeLeft);
      if (timeLeft <= 0) endRound();
    }, 1000);
  }

  function allInputsFilled() {
    return Array.from(answerInputs).every((input) => input.value.trim().length > 0);
  }

  function updateStopButtonState() {
    if (!stopBtn) return;
    const canStop = roundRunning && allInputsFilled();
    stopBtn.classList.toggle("disabled", !canStop);
  }



  answerInputs.forEach(function (input) {
    input.addEventListener("input", function () {
      showStopWarning(false);
      updateStopButtonState();
    });
  });

  function endRound() {
    if (!roundRunning) return;

    roundRunning = false;
    if (timerId !== null) {
      clearInterval(timerId);
      timerId = null;
    }

    setInputsEnabled(false);

    const answers = {};
    let validCount = 0;
    let roundScore = 0;

    const entries = Array.from(answerInputs).map((input) => {
      const cat = input.dataset.category;
      const value = input.value.trim();
      answers[cat] = value;

      let ok = false;
      if (cat) ok = isValidWordForCategoryAndLetter(cat, value, currentLetter);

      const norm = String(value || "").trim().toLowerCase();

      return { input, cat, value, ok, norm };
    });

    const counts = {};
    entries.forEach((e) => {
      if (!e.value) return;
      if (!e.ok) return;
      if (!e.norm) return;
      counts[e.norm] = (counts[e.norm] || 0) + 1;
    });

    entries.forEach((e) => {
      let points = 0;

      if (!e.value || !e.ok) {
        points = 0;
      } else if (counts[e.norm] > 1) {
        points = 10; 
        validCount++;
      } else {
        points = 20; 
        validCount++;
      }

      roundScore += points;

      e.input.classList.toggle("valida", e.ok && !!e.value);
      e.input.classList.toggle("erro", !e.ok || !e.value);
    });

    updateStopButtonState();

    totalScore += roundScore;

    if (isOnline && window.api && window.api.submit) {
      window.api.submit(roomId, answers, roundScore, function (res) {
        if (!res || !res.ok) console.warn("submit falhou:", res);
      });
    }

    if (scoreOverlay && scoreValueSpan && validCountSpan && totalCountSpan && totalScoreSpan) {
      scoreValueSpan.textContent = String(roundScore);
      validCountSpan.textContent = String(validCount);
      totalCountSpan.textContent = String(answerInputs.length);
      totalScoreSpan.textContent = String(totalScore);

      scoreOverlay.classList.add("show");
    }
  }


  function fancyEndTransition() {
    const overlay = document.getElementById("end-transition");
    if (overlay) overlay.classList.add("show");

    setTimeout(function () {
      window.location.href = "../index.html";
    }, 1800);
  }

  if (startBtn) {
    startBtn.addEventListener("click", function () {
      resetRoundUI();

      if (!isOnline) {
        startRoundOffline();
        return;
      }

      if (window.api && window.api.startRound) {
        window.api.startRound(roomId, function (res) {
          if (!res || !res.ok) {
            alert(res && res.error ? res.error : "Não deu para iniciar o round.");
          }
        });
      } else {
        alert("API online não está carregada. Confirma os <script> no game.html.");
      }
    });
  }

  if (stopBtn) {
    stopBtn.addEventListener("click", function () {
      if (!roundRunning) return;
      if (stopRequested) return;

      if (isOnline && window.api && window.api.stopRound) {
        if (!allInputsFilled()) {
          showStopWarning(true);
          return;
        }
        showStopWarning(false);
        stopRequested = true; 
        window.api.stopRound(roomId, function (res) {
          if (!res || !res.ok) {
            if (res && res.error === "Round já terminou.") return;

            stopRequested = false;
            alert((res && res.error) ? res.error : "Não foi possível parar o round.");
          }
        });
      } else {
        endRound();
      }
    });
  }

  if (closeScoreBtn) {
    closeScoreBtn.addEventListener("click", function () {
      if (currentRound === total_rounds) {
        scoreOverlay.classList.remove("show");
        fancyEndTransition();
        return;
      }
      scoreOverlay.classList.remove("show");
    });
  }

  function clearInputsUI() {
    answerInputs.forEach(function (input) {
      input.classList.remove("valida", "erro");
      input.value = "";
    });
  }



  function handleRoundStarted(payload) {
    showStopWarning(false);
    stopRequested = false;

    clearInputsUI();

    currentRound = payload.round;
    total_rounds = payload.rounds;
    currentLetter = payload.letter;

    if (roundSpan) roundSpan.textContent = currentRound + "/" + total_rounds;
    if (letterSpan) letterSpan.textContent = currentLetter;

    roundRunning = true;
    setInputsEnabled(true);
    updateStopButtonState();

    if (timerId !== null) clearInterval(timerId);

    function tick() {
      const elapsed = Math.floor((Date.now() - payload.roundStartAt) / 1000);
      const left = Math.max(0, payload.timePerRound - elapsed);
      timeLeft = left;
      if (timerSpan) timerSpan.textContent = String(left);
        if (left <= 0) {
          if (isOnline && window.api && window.api.stopRound) {
            stopRequested = true;
            clearInterval(timerId);
            timerId = null;
            
            if (!roundRunning) return;

            endRound();
          }
        }
    }

    tick();
    timerId = setInterval(tick, 250);
  }


  if (isOnline) {
    if (!window.api || !window.api.joinRoom || !window.socket) {
      alert("Online não está carregado. Confirma os <script> no game.html.");
    } else {
      const savedPassword = localStorage.getItem("roomPassword") || ""; 

      window.api.joinRoom(
        { roomId: roomId, nickname: nickname, avatar: avatar, password: savedPassword },
        function (res) {
          if (!res || !res.ok) {
            alert(res && res.error ? res.error : "Erro ao entrar na sala.");
            window.location.href = "../index.html";
            return;
          }

          window.socket.emit("game:getState", { roomId: roomId }, function (stateRes) {
            if (!stateRes || !stateRes.ok || !stateRes.state) return;

            if (
              stateRes.state.running &&
              stateRes.state.letter &&
              stateRes.state.roundStartAt
            ) {
              handleRoundStarted({
                round: stateRes.state.round,
                rounds: stateRes.config.rounds,
                letter: stateRes.state.letter,
                timePerRound: stateRes.config.timePerRound,
                roundStartAt: stateRes.state.roundStartAt
              });
            }
          });
        }
      );

      window.socket.on("game:roundStarted", function (payload) {
        isCountingNextRound = false;

        if (nextRoundCountdownTimer) {
          clearInterval(nextRoundCountdownTimer);
          nextRoundCountdownTimer = null;
        }

        handleRoundStarted(payload);
        if (compareOverlay) compareOverlay.classList.add("hidden");
        if (compareState) compareState.open = false;
      });


      window.socket.on("game:forceEndRound", function() {
        endRound();
      });

      window.socket.on("game:roundEnded", function (payload) {
        openCompareOverlay(payload || {});
      });

      window.socket.on("game:nextRoundStarting", function (payload) {
        if (!compareReadyText || !payload || !payload.startAt) return;

        isCountingNextRound = true;

        if (nextRoundCountdownTimer) {
          clearInterval(nextRoundCountdownTimer);
          nextRoundCountdownTimer = null;
        }

        function updateCountdown() {
          const msLeft = payload.startAt - Date.now();
          const secondsLeft = Math.max(0, Math.ceil(msLeft / 1000));

          if (secondsLeft > 0) {
            compareReadyText.textContent = "A próxima ronda começa em " + secondsLeft + "s...";
          } else {
            compareReadyText.textContent = "A começar...";
            clearInterval(nextRoundCountdownTimer);
            nextRoundCountdownTimer = null;
          }
        }

        updateCountdown();
        nextRoundCountdownTimer = setInterval(updateCountdown, 250);
      });


      window.socket.on("game:nextReadyUpdate", function (payload) {
        if (!compareReadyText) return;
        if (!payload) return;
        if (isCountingNextRound) return;

        compareReadyText.textContent = "Prontos: " + payload.ready + "/" + payload.total;
      });

      window.socket.on("game:showResults", function(payload) {
        if (!payload) return;
        show_results(payload);
        if (localStorage.getItem("roomPassword")) {
          localStorage.removeItem("roomPassword");
        }
      })
    }
  }

  resetRoundUI();
});
