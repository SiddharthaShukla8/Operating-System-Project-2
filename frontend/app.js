const sentenceWrapper = document.getElementById('sentence-wrapper');
const cursor = document.getElementById('cursor');
const timerDisplay = document.getElementById('timer-display');
const restartBtn = document.getElementById('restart-btn');
const playAgainBtn = document.getElementById('play-again-btn');
const gameView = document.getElementById('game-view');
const resultView = document.getElementById('result-view');
const commandLine = document.getElementById('command-line');
const commandInput = document.getElementById('command-input');

let state = {
    words: [],
    currentWordIndex: 0,
    currentCharIndex: 0,
    startTime: null,
    stats: { correct: 0, incorrect: 0, extra: 0, missed: 0, raw: 0 },
    history: { wpm: [], raw: [], labels: [] },
    isFinished: false,
    timerInterval: null,
    selectedTime: 30,
    selectedWords: 25,
    timeLeft: 30,
    mode: "time",
    punctuation: false,
    numbers: false
};

let chart = null;

async function fetchSentence() {
    let url = `/api/sentence?mode=${state.mode}`;
    if (state.mode === 'words') url += `&count=${state.selectedWords}`;
    if (state.punctuation) url += `&punctuation=true`;
    if (state.numbers) url += `&numbers=true`;

    const response = await fetch(url);
    const data = await response.json();
    state.words = data.sentence.split(' ');
    renderSentence();
}

function renderSentence() {
    sentenceWrapper.innerHTML = '';
    state.words.forEach((word, wIdx) => {
        const wordSpan = document.createElement('div');
        wordSpan.className = 'word';
        word.split('').forEach(char => {
            const span = document.createElement('span');
            span.innerText = char;
            wordSpan.appendChild(span);
        });
        sentenceWrapper.appendChild(wordSpan);
    });
    updateCursor();
}

function updateCursor() {
    const words = sentenceWrapper.querySelectorAll('.word');
    const word = words[state.currentWordIndex];
    if (!word) return;
    const spans = word.querySelectorAll('span');
    
    let targetSpan;
    if (state.currentCharIndex < spans.length) {
        targetSpan = spans[state.currentCharIndex];
    } else {
        targetSpan = spans[spans.length - 1];
    }

    const rect = targetSpan.getBoundingClientRect();
    const wrapperRect = sentenceWrapper.getBoundingClientRect();
    
    if (state.currentCharIndex >= spans.length) {
        cursor.style.left = `${rect.right - wrapperRect.left}px`;
    } else {
        cursor.style.left = `${rect.left - wrapperRect.left}px`;
    }
    cursor.style.top = `${rect.top - wrapperRect.top}px`;
}

function handleInput(e) {
    if (state.isFinished || !commandLine.classList.contains('hidden')) return;

    if (!state.startTime) {
        state.startTime = Date.now();
        if (state.mode === 'time') startTimer();
        if (state.mode === 'zen') {
            document.getElementById('zen-tooltip').classList.remove('hidden');
            timerDisplay.innerText = "zen";
        }
    }

    const key = e.key;
    const words = sentenceWrapper.querySelectorAll('.word');
    const currentWordSpan = words[state.currentWordIndex];
    if (!currentWordSpan) return;
    const spans = currentWordSpan.querySelectorAll('span');

    if (key === ' ') {
        e.preventDefault();
        for (let i = state.currentCharIndex; i < spans.length; i++) {
            spans[i].classList.add('missed');
            state.stats.missed++;
        }
        state.currentWordIndex++;
        state.currentCharIndex = 0;
        if (state.currentWordIndex === state.words.length) finishSession();
        else updateCursor();
        return;
    }

    if (key === 'Backspace') {
        if (state.currentCharIndex > 0) {
            state.currentCharIndex--;
            const span = spans[state.currentCharIndex];
            if (span.classList.contains('extra')) {
                span.remove();
                state.stats.extra--;
            } else {
                if (span.classList.contains('correct')) state.stats.correct--;
                if (span.classList.contains('incorrect')) state.stats.incorrect--;
                span.className = '';
            }
            updateCursor();
        }
        return;
    }

    if (key.length !== 1) return;

    state.stats.raw++;
    if (state.currentCharIndex < spans.length) {
        const targetChar = state.words[state.currentWordIndex][state.currentCharIndex];
        if (key === targetChar) {
            spans[state.currentCharIndex].className = 'correct';
            state.stats.correct++;
        } else {
            spans[state.currentCharIndex].className = 'incorrect';
            state.stats.incorrect++;
        }
        state.currentCharIndex++;
    } else {
        const extraSpan = document.createElement('span');
        extraSpan.innerText = key;
        extraSpan.className = 'incorrect extra';
        currentWordSpan.appendChild(extraSpan);
        state.currentCharIndex++;
        state.stats.extra++;
    }

    updateCursor();
}

function startTimer() {
    timerDisplay.innerText = state.timeLeft;
    state.timerInterval = setInterval(() => {
        state.timeLeft--;
        timerDisplay.innerText = state.timeLeft;
        
        const elapsed = (Date.now() - state.startTime) / 1000 / 60;
        const currentWpm = Math.round((state.stats.correct / 5) / (elapsed || 1));
        const currentRaw = Math.round((state.stats.raw / 5) / (elapsed || 1));
        state.history.wpm.push(currentWpm);
        state.history.raw.push(currentRaw);
        state.history.labels.push(state.history.labels.length + 1);

        if (state.timeLeft <= 0) finishSession();
    }, 1000);
}

function finishSession() {
    state.isFinished = true;
    clearInterval(state.timerInterval);
    
    const elapsed = (Date.now() - state.startTime) / 1000 / 60;
    const wpm = Math.round((state.stats.correct / 5) / (elapsed || 1));
    const acc = Math.round((state.stats.correct / (state.stats.correct + state.stats.incorrect + state.stats.extra)) * 100);

    document.getElementById('final-wpm').innerText = wpm;
    document.getElementById('final-acc').innerText = acc + '%';
    document.getElementById('res-mode').innerText = state.mode;
    document.getElementById('res-time').innerText = (state.mode === 'time' ? state.selectedTime : state.selectedWords);
    document.getElementById('res-raw').innerText = Math.round(state.stats.raw / (elapsed || 1));
    document.getElementById('res-chars').innerText = `${state.stats.correct}/${state.stats.incorrect}/${state.stats.extra}/${state.stats.missed}`;

    renderChart();
    gameView.classList.add('hidden');
    resultView.classList.remove('hidden');
}

function renderChart() {
    const ctx = document.getElementById('performance-chart').getContext('2d');
    if (chart) chart.destroy();
    chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: state.history.labels,
            datasets: [{
                label: 'wpm',
                data: state.history.wpm,
                borderColor: '#e2b714',
                backgroundColor: 'rgba(226, 183, 20, 0.1)',
                fill: true,
                tension: 0.4
            }, {
                label: 'raw',
                data: state.history.raw,
                borderColor: '#646669',
                borderDash: [5, 5],
                fill: false,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: { beginAtZero: true, grid: { color: '#323437' } },
                x: { grid: { color: '#323437' } }
            },
            plugins: { legend: { display: false } }
        }
    });
}

function restart() {
    clearInterval(state.timerInterval);
    state = { ...state, currentWordIndex: 0, currentCharIndex: 0, startTime: null, stats: { correct: 0, incorrect: 0, extra: 0, missed: 0, raw: 0 }, history: { wpm: [], raw: [], labels: [] }, isFinished: false, timerInterval: null, timeLeft: state.selectedTime };
    timerDisplay.innerText = state.mode === 'words' ? state.selectedWords : (state.mode === 'zen' ? 'zen' : state.selectedTime);
    gameView.classList.remove('hidden');
    resultView.classList.add('hidden');
    commandLine.classList.add('hidden');
    document.getElementById('zen-tooltip').classList.add('hidden');
    fetchSentence();
}

// Logic for all options and buttons
document.querySelectorAll('#modes span').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('#modes span').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        state.mode = btn.dataset.mode;
        
        document.getElementById('time-options').classList.toggle('hidden', state.mode !== 'time');
        document.getElementById('word-options').classList.toggle('hidden', state.mode !== 'words');
        
        restart();
    });
});

document.querySelectorAll('.options span').forEach(btn => {
    btn.addEventListener('click', () => {
        const parent = btn.parentElement.id;
        document.querySelectorAll(`#${parent} span`).forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        if (parent === 'time-options') state.selectedTime = parseInt(btn.dataset.value);
        if (parent === 'word-options') state.selectedWords = parseInt(btn.dataset.value);
        restart();
    });
});

document.getElementById('toggle-punctuation').addEventListener('click', function() {
    this.classList.toggle('active');
    state.punctuation = !state.punctuation;
    restart();
});

document.getElementById('toggle-numbers').addEventListener('click', function() {
    this.classList.toggle('active');
    state.numbers = !state.numbers;
    restart();
});

window.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') { commandLine.classList.toggle('hidden'); if (!commandLine.classList.contains('hidden')) commandInput.focus(); }
    if (e.key === 'Tab') { e.preventDefault(); restart(); }
    if (e.key === 'Enter' && e.shiftKey && state.mode === 'zen') finishSession();
    handleInput(e);
});

restartBtn.addEventListener('click', restart);
playAgainBtn.addEventListener('click', restart);
document.querySelector('.modal-bg').addEventListener('click', () => commandLine.classList.add('hidden'));

fetchSentence();

window.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') { commandLine.classList.toggle('hidden'); if (!commandLine.classList.contains('hidden')) commandInput.focus(); }
    if (e.key === 'Tab') { e.preventDefault(); restart(); }
    if (e.key === 'Enter' && e.shiftKey && state.mode === 'zen') finishSession();
    handleInput(e);
});

restartBtn.addEventListener('click', restart);
playAgainBtn.addEventListener('click', restart);
fetchSentence();
