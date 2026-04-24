const sentenceWrapper = document.getElementById('sentence-wrapper');
const cursor = document.getElementById('cursor');
const timerDisplay = document.getElementById('timer');
const wpmDisplay = document.getElementById('wpm');
const accDisplay = document.getElementById('accuracy');
const restartBtn = document.getElementById('restart-btn');
const resultsScreen = document.getElementById('results');

let state = {
    sentence: "",
    currentIndex: 0,
    startTime: null,
    errors: 0,
    totalTyped: 0,
    isFinished: false,
    timerInterval: null,
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
            spans[state.currentIndex].className = '';
            updateCursor();
        }
        return;
    }

    if (key.length !== 1) return;

    const targetChar = state.sentence[state.currentIndex];
    
    if (key === targetChar) {
        spans[state.currentIndex].className = 'correct';
    } else {
        spans[state.currentIndex].className = 'incorrect';
        state.errors++;
    }

    state.currentIndex++;
    state.totalTyped++;
    updateCursor();
    updateStats();

    if (state.currentIndex === state.sentence.length) {
        finishSession();
    }
}

function startTimer() {
    state.timerInterval = setInterval(() => {
        state.timeLeft--;
        timerDisplay.innerText = state.timeLeft;
        if (state.timeLeft <= 0) {
            finishSession();
        }
    }, 1000);
}

function updateStats() {
    const elapsed = (Date.now() - state.startTime) / 1000 / 60; // in minutes
    const wpm = Math.round((state.totalTyped / 5) / (elapsed || 1));
    const acc = Math.round(((state.totalTyped - state.errors) / state.totalTyped) * 100);

    wpmDisplay.innerText = wpm;
    accDisplay.innerText = acc;
}

function finishSession() {
    state.isFinished = true;
    clearInterval(state.timerInterval);
    
    document.getElementById('final-wpm').innerText = wpmDisplay.innerText;
    document.getElementById('final-acc').innerText = accDisplay.innerText + '%';
    resultsScreen.classList.remove('hidden');
}

function restart() {
    clearInterval(state.timerInterval);
    state = {
        sentence: "",
        currentIndex: 0,
        startTime: null,
        errors: 0,
        totalTyped: 0,
        isFinished: false,
        timerInterval: null,
        timeLeft: 30
    };
    timerDisplay.innerText = "30";
    wpmDisplay.innerText = "0";
    accDisplay.innerText = "100";
    resultsScreen.classList.add('hidden');
    fetchSentence();
}

window.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
        restart();
    } else {
        handleInput(e);
    }
});

restartBtn.addEventListener('click', restart);
document.getElementById('play-again').addEventListener('click', restart);

fetchSentence();
