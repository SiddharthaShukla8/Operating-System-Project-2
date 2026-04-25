const sentenceWrapper = document.getElementById('sentence-wrapper');
const cursor = document.getElementById('cursor');
const timerDisplay = document.getElementById('timer-display');
const restartBtn = document.getElementById('restart-btn');
const playAgainBtn = document.getElementById('play-again-btn');
const gameView = document.getElementById('game-view');
const resultView = document.getElementById('result-view');

let state = {
    sentence: "",
    currentIndex: 0,
    startTime: null,
    stats: {
        correct: 0,
        incorrect: 0,
        extra: 0,
        missed: 0
    },
    totalTyped: 0,
    isFinished: false,
    timerInterval: null,
    selectedTime: 30,
    timeLeft: 30
};

async function fetchSentence() {
    const response = await fetch('/api/sentence');
    const data = await response.json();
    state.sentence = data.sentence;
    renderSentence();
}

function renderSentence() {
    sentenceWrapper.innerHTML = '';
    state.sentence.split('').forEach(char => {
        const span = document.createElement('span');
        span.innerText = char;
        sentenceWrapper.appendChild(span);
    });
    updateCursor();
}

function updateCursor() {
    const spans = sentenceWrapper.querySelectorAll('span');
    if (state.currentIndex < spans.length) {
        const rect = spans[state.currentIndex].getBoundingClientRect();
        const wrapperRect = sentenceWrapper.getBoundingClientRect();
        cursor.style.left = `${rect.left - wrapperRect.left}px`;
        cursor.style.top = `${rect.top - wrapperRect.top}px`;
        cursor.classList.remove('hidden');
    } else {
        cursor.classList.add('hidden');
    }
}

function handleInput(e) {
    if (state.isFinished) return;

    if (!state.startTime) {
        state.startTime = Date.now();
        startTimer();
    }

    const key = e.key;
    const spans = sentenceWrapper.querySelectorAll('span');

    if (key === 'Backspace') {
        if (state.currentIndex > 0) {
            state.currentIndex--;
            const span = spans[state.currentIndex];
            if (span.classList.contains('correct')) state.stats.correct--;
            if (span.classList.contains('incorrect')) state.stats.incorrect--;
            span.className = '';
            updateCursor();
        }
        return;
    }

    if (key.length !== 1) return;

    if (state.currentIndex >= state.sentence.length) return;

    const targetChar = state.sentence[state.currentIndex];
    const span = spans[state.currentIndex];
    
    if (key === targetChar) {
        span.className = 'correct';
        state.stats.correct++;
    } else {
        span.className = 'incorrect';
        state.stats.incorrect++;
    }

    state.currentIndex++;
    state.totalTyped++;
    updateCursor();

    if (state.currentIndex === state.sentence.length) {
        finishSession();
    }
}

function startTimer() {
    timerDisplay.innerText = state.timeLeft;
    state.timerInterval = setInterval(() => {
        state.timeLeft--;
        timerDisplay.innerText = state.timeLeft;
        if (state.timeLeft <= 0) {
            finishSession();
        }
    }, 1000);
}

function finishSession() {
    state.isFinished = true;
    clearInterval(state.timerInterval);
    
    const elapsed = (Date.now() - state.startTime) / 1000 / 60; // in minutes
    const wpm = Math.round((state.stats.correct / 5) / (elapsed || 1));
    const totalPossible = state.totalTyped;
    const acc = totalPossible > 0 ? Math.round((state.stats.correct / totalPossible) * 100) : 0;

    document.getElementById('final-wpm').innerText = wpm;
    document.getElementById('final-acc').innerText = acc + '%';
    document.getElementById('res-time').innerText = state.selectedTime + 's';
    
    state.stats.missed = state.sentence.length - state.currentIndex;
    document.getElementById('res-chars').innerText = `${state.stats.correct}/${state.stats.incorrect}/${state.stats.extra}/${state.stats.missed}`;

    gameView.classList.add('hidden');
    resultView.classList.remove('hidden');
}

function restart() {
    clearInterval(state.timerInterval);
    state = {
        ...state,
        currentIndex: 0,
        startTime: null,
        stats: { correct: 0, incorrect: 0, extra: 0, missed: 0 },
        totalTyped: 0,
        isFinished: false,
        timerInterval: null,
        timeLeft: state.selectedTime
    };
    timerDisplay.innerText = state.selectedTime;
    gameView.classList.remove('hidden');
    resultView.classList.add('hidden');
    fetchSentence();
}

window.addEventListener('keydown', (e) => {
    if (e.key === 'Tab') {
        e.preventDefault(); // Prevent tab focus
    }
    
    if (e.key === 'Enter' || (e.key === 'r' && e.ctrlKey)) {
        restart();
    } else {
        handleInput(e);
    }
});

restartBtn.addEventListener('click', restart);
playAgainBtn.addEventListener('click', restart);

document.querySelectorAll('#time-selection span').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('#time-selection span').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        state.selectedTime = parseInt(btn.dataset.time);
        restart();
    });
});

fetchSentence();
