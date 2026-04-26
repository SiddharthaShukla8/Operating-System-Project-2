const WORD_FETCH_LIMIT = 20;
const TIME_MODE_WORD_BUFFER = 120;
const ZEN_MODE_WORD_BUFFER = 160;
const DEFAULT_CUSTOM_TEXT = 'Operating systems reward precision, calm hands, and a typing rhythm that stays accurate under pressure.';
const FALLBACK_WORD_POOL = [
    'kernel', 'memory', 'screen', 'typing', 'system', 'input', 'output', 'buffer',
    'process', 'thread', 'timing', 'logic', 'signal', 'render', 'cursor', 'report',
    'typing', 'custom', 'quote', 'speed', 'accuracy', 'debug', 'module', 'library',
    'screen', 'keyboard', 'session', 'result', 'minimal', 'stable', 'project', 'track'
];

const elements = {
    timerDisplay: document.getElementById('timer-display'),
    liveWpm: document.getElementById('live-wpm'),
    liveRaw: document.getElementById('live-raw'),
    liveAcc: document.getElementById('live-acc'),
    modeSummary: document.getElementById('mode-summary'),
    sessionTitle: document.getElementById('session-title'),
    promptLabel: document.getElementById('prompt-label'),
    progressLabel: document.getElementById('progress-label'),
    sentenceWrapper: document.getElementById('sentence-wrapper'),
    cursor: document.getElementById('cursor'),
    typingContainer: document.getElementById('typing-container'),
    gameView: document.getElementById('game-view'),
    resultView: document.getElementById('result-view'),
    restartBtn: document.getElementById('restart-btn'),
    playAgainBtn: document.getElementById('play-again-btn'),
    customPanel: document.getElementById('custom-panel'),
    customText: document.getElementById('custom-text'),
    applyCustomBtn: document.getElementById('apply-custom-btn'),
    togglePunctuation: document.getElementById('toggle-punctuation'),
    toggleNumbers: document.getElementById('toggle-numbers'),
    toggleSound: document.getElementById('toggle-sound')
};

const state = {
    mode: 'time',
    selectedTime: 30,
    selectedWords: 25,
    selectedQuoteWords: 30,
    punctuation: false,
    numbers: false,
    soundEnabled: false,
    customText: DEFAULT_CUSTOM_TEXT,
    words: [],
    inputs: [],
    submitted: [],
    currentWordIndex: 0,
    currentCharIndex: 0,
    startTime: null,
    timeLeft: 30,
    isFinished: false,
    timerInterval: null,
    history: {
        labels: [],
        wpm: [],
        raw: []
    },
    stats: {
        correctChars: 0,
        incorrectChars: 0,
        totalTypedChars: 0
    }
};

let chart = null;
let audioContext = null;

function sanitizeCustomText(text) {
    return text
        .trim()
        .replace(/\s+/g, ' ')
        .split(' ')
        .filter(Boolean);
}

function getModeSummary() {
    if (state.mode === 'time') return `time ${state.selectedTime}s`;
    if (state.mode === 'words') return `words ${state.selectedWords}`;
    if (state.mode === 'quote') return `quote ${state.selectedQuoteWords} words`;
    if (state.mode === 'custom') return 'custom text';
    return 'zen mode';
}

function getPromptTitle() {
    if (state.mode === 'time') return 'Time challenge';
    if (state.mode === 'words') return 'Word challenge';
    if (state.mode === 'quote') return 'Quote challenge';
    if (state.mode === 'custom') return 'Custom practice';
    return 'Zen practice';
}

function getTargetWordCount() {
    if (state.mode === 'time') return TIME_MODE_WORD_BUFFER;
    if (state.mode === 'words') return state.selectedWords;
    if (state.mode === 'quote') return state.selectedQuoteWords;
    if (state.mode === 'custom') return sanitizeCustomText(elements.customText.value || state.customText).length;
    return ZEN_MODE_WORD_BUFFER;
}

function buildFallbackWords(count) {
    const words = [];
    for (let index = 0; index < count; index += 1) {
        words.push(FALLBACK_WORD_POOL[index % FALLBACK_WORD_POOL.length]);
    }
    return words;
}

async function fetchWordChunk(count) {
    const params = new URLSearchParams();
    params.set('mode', state.mode);
    params.set('count', String(count));
    if (state.punctuation) params.set('punctuation', 'true');
    if (state.numbers) params.set('numbers', 'true');

    const response = await fetch(`/api/sentence?${params.toString()}`);
    if (!response.ok) {
        throw new Error(`Prompt request failed with status ${response.status}`);
    }

    const data = await response.json();
    return String(data.sentence || '')
        .split(' ')
        .filter(Boolean);
}

async function fetchWords(totalCount) {
    const words = [];

    while (words.length < totalCount) {
        const remaining = totalCount - words.length;
        const chunkSize = remaining > WORD_FETCH_LIMIT ? WORD_FETCH_LIMIT : remaining;
        const chunk = await fetchWordChunk(chunkSize);

        if (!chunk.length) break;
        words.push(...chunk);
    }

    if (!words.length) {
        return buildFallbackWords(totalCount);
    }

    if (words.length < totalCount) {
        words.push(...buildFallbackWords(totalCount - words.length));
    }

    return words.slice(0, totalCount);
}

async function buildPromptWords() {
    if (state.mode === 'custom') {
        const source = elements.customText.value.trim() || state.customText || DEFAULT_CUSTOM_TEXT;
        const customWords = sanitizeCustomText(source);
        return customWords.length ? customWords : sanitizeCustomText(DEFAULT_CUSTOM_TEXT);
    }

    try {
        const words = await fetchWords(getTargetWordCount());
        if (state.mode === 'quote' && words.length > 0) {
            words[0] = words[0].charAt(0).toUpperCase() + words[0].slice(1);
            words[words.length - 1] = `${words[words.length - 1]}.`;
        }
        return words;
    } catch (error) {
        const fallback = buildFallbackWords(getTargetWordCount());
        if (state.mode === 'quote' && fallback.length > 0) {
            fallback[0] = fallback[0].charAt(0).toUpperCase() + fallback[0].slice(1);
            fallback[fallback.length - 1] = `${fallback[fallback.length - 1]}.`;
        }
        return fallback;
    }
}

function resetRuntimeState() {
    clearInterval(state.timerInterval);
    state.inputs = Array.from({ length: state.words.length }, () => '');
    state.submitted = Array.from({ length: state.words.length }, () => false);
    state.currentWordIndex = 0;
    state.currentCharIndex = 0;
    state.startTime = null;
    state.timeLeft = state.selectedTime;
    state.isFinished = false;
    state.timerInterval = null;
    state.history = { labels: [], wpm: [], raw: [] };
    state.stats = { correctChars: 0, incorrectChars: 0, totalTypedChars: 0 };
}

function updateOptionVisibility() {
    document.getElementById('time-options').classList.toggle('hidden', state.mode !== 'time');
    document.getElementById('word-options').classList.toggle('hidden', state.mode !== 'words');
    document.getElementById('quote-options').classList.toggle('hidden', state.mode !== 'quote');
    elements.customPanel.classList.toggle('hidden', state.mode !== 'custom');
}

function updateHeadings() {
    elements.modeSummary.textContent = getModeSummary();
    elements.sessionTitle.textContent = getPromptTitle();
    elements.promptLabel.textContent = state.mode === 'custom'
        ? 'your custom paragraph is ready'
        : `prompt loaded for ${getModeSummary()}`;
}

function updateSoundToggle() {
    elements.toggleSound.classList.toggle('active', state.soundEnabled);
    elements.toggleSound.setAttribute('aria-pressed', String(state.soundEnabled));
    elements.toggleSound.textContent = state.soundEnabled ? 'sound on' : 'sound off';
}

function focusTypingArea() {
    if (document.activeElement instanceof HTMLElement && document.activeElement !== elements.typingContainer) {
        document.activeElement.blur();
    }

    elements.typingContainer.focus({ preventScroll: true });
    window.requestAnimationFrame(() => {
        elements.typingContainer.focus({ preventScroll: true });
    });
}

function ensureAudioContext() {
    if (!state.soundEnabled) return null;

    const AudioContextClass = window.AudioContext || window.webkitAudioContext;
    if (!AudioContextClass) return null;

    if (!audioContext) {
        audioContext = new AudioContextClass();
    }

    if (audioContext.state === 'suspended') {
        audioContext.resume().catch(() => {});
    }

    return audioContext;
}

function playSound(type) {
    const context = ensureAudioContext();
    if (!context) return;

    const oscillator = context.createOscillator();
    const gain = context.createGain();
    oscillator.type = type === 'incorrect' ? 'square' : 'sine';
    oscillator.frequency.value = type === 'incorrect' ? 180 : type === 'action' ? 420 : 280;
    gain.gain.value = type === 'incorrect' ? 0.04 : 0.025;

    oscillator.connect(gain);
    gain.connect(context.destination);

    const now = context.currentTime;
    gain.gain.setValueAtTime(gain.gain.value, now);
    gain.gain.exponentialRampToValueAtTime(0.0001, now + 0.08);
    oscillator.start(now);
    oscillator.stop(now + 0.08);
}

function countSubmittedMissedChars() {
    let missed = 0;

    state.words.forEach((word, index) => {
        if (!state.submitted[index]) return;

        const typedLength = (state.inputs[index] || '').length;
        if (typedLength < word.length) {
            missed += word.length - typedLength;
        }
    });

    return missed;
}

function updateLiveStats() {
    const liveMissedChars = countSubmittedMissedChars();
    const accuracyBase = state.stats.totalTypedChars + liveMissedChars;
    const accuracy = accuracyBase
        ? Math.round((state.stats.correctChars / accuracyBase) * 100)
        : 100;

    let wpm = 0;
    let raw = 0;

    if (state.startTime) {
        const elapsedMinutes = Math.max((Date.now() - state.startTime) / 60000, 1 / 60);
        wpm = Math.round((state.stats.correctChars / 5) / elapsedMinutes);
        raw = Math.round((state.stats.totalTypedChars / 5) / elapsedMinutes);
    }

    elements.liveWpm.textContent = String(wpm);
    elements.liveRaw.textContent = String(raw);
    elements.liveAcc.textContent = `${accuracy}%`;
}

function updateProgressLabel() {
    if (state.mode === 'time' || state.mode === 'zen') {
        const clearedWords = state.submitted.filter(Boolean).length;
        elements.progressLabel.textContent = `${clearedWords} words cleared`;
        return;
    }

    const total = state.words.length;
    const current = total ? Math.min(state.currentWordIndex + 1, total) : 0;
    elements.progressLabel.textContent = `word ${current} / ${total}`;
}

function renderWords() {
    const fragment = document.createDocumentFragment();

    state.words.forEach((word, wordIndex) => {
        const wordElement = document.createElement('div');
        wordElement.className = 'word';

        const typedWord = state.inputs[wordIndex] || '';

        word.split('').forEach((character, charIndex) => {
            const charElement = document.createElement('span');
            charElement.className = 'char';
            charElement.textContent = character;

            const typedChar = typedWord[charIndex];
            if (typedChar !== undefined) {
                charElement.classList.add(typedChar === character ? 'correct' : 'incorrect');
            } else if (state.submitted[wordIndex]) {
                charElement.classList.add('missed');
            }

            wordElement.appendChild(charElement);
        });

        fragment.appendChild(wordElement);
    });

    elements.sentenceWrapper.innerHTML = '';
    elements.sentenceWrapper.appendChild(fragment);
    updateProgressLabel();
    updateCursor();
}

function updateCursor() {
    if (state.isFinished) {
        elements.cursor.classList.add('hidden');
        return;
    }

    const wordElements = elements.sentenceWrapper.querySelectorAll('.word');
    const currentWord = wordElements[state.currentWordIndex];
    if (!currentWord) return;

    const charElements = currentWord.querySelectorAll('.char');
    const wordText = state.words[state.currentWordIndex];
    const wordLength = wordText.length;
    const safeIndex = state.currentCharIndex < wordLength ? state.currentCharIndex : wordLength - 1;
    const targetChar = charElements[safeIndex];
    if (!targetChar) return;

    const containerRect = elements.typingContainer.getBoundingClientRect();
    const charRect = targetChar.getBoundingClientRect();

    let left = charRect.left - containerRect.left;
    if (state.currentCharIndex >= wordLength) {
        left = charRect.right - containerRect.left + 1;
    }

    const top = charRect.top - containerRect.top;

    elements.cursor.classList.remove('hidden');
    elements.cursor.style.left = `${left}px`;
    elements.cursor.style.top = `${top}px`;
}

function markSnapshot() {
    if (!state.startTime || state.isFinished) return;

    const elapsedSeconds = Math.max(1, Math.floor((Date.now() - state.startTime) / 1000));
    const elapsedMinutes = Math.max((Date.now() - state.startTime) / 60000, 1 / 60);

    state.history.labels.push(`${elapsedSeconds}s`);
    state.history.wpm.push(Math.round((state.stats.correctChars / 5) / elapsedMinutes));
    state.history.raw.push(Math.round((state.stats.totalTypedChars / 5) / elapsedMinutes));
}

function ensureStarted() {
    if (state.startTime) return;

    state.startTime = Date.now();
    if (state.mode === 'time') {
        elements.timerDisplay.textContent = String(state.timeLeft);
    } else if (state.mode === 'zen') {
        elements.timerDisplay.textContent = 'zen';
    } else {
        elements.timerDisplay.textContent = 'run';
    }

    state.timerInterval = setInterval(() => {
        if (state.isFinished) return;

        if (state.mode === 'time') {
            state.timeLeft -= 1;
            elements.timerDisplay.textContent = String(Math.max(0, state.timeLeft));
            if (state.timeLeft <= 0) {
                finishSession();
                return;
            }
        }

        markSnapshot();
        updateLiveStats();
    }, 1000);
}

function updateStatsForRemoval(removedChar, expectedChar) {
    state.stats.totalTypedChars = Math.max(0, state.stats.totalTypedChars - 1);
    if (removedChar === expectedChar) {
        state.stats.correctChars = Math.max(0, state.stats.correctChars - 1);
    } else {
        state.stats.incorrectChars = Math.max(0, state.stats.incorrectChars - 1);
    }
}

function moveToPreviousWord() {
    if (state.currentWordIndex === 0) return;

    const previousIndex = state.currentWordIndex - 1;
    if (!state.submitted[previousIndex]) return;

    state.submitted[previousIndex] = false;
    state.currentWordIndex = previousIndex;
    state.currentCharIndex = state.inputs[previousIndex].length;
    playSound('action');
    renderWords();
}

function typeCharacter(key) {
    const currentWord = state.words[state.currentWordIndex];
    if (!currentWord) return;

    if (state.currentCharIndex >= currentWord.length) {
        playSound('incorrect');
        return;
    }

    const expectedChar = currentWord[state.currentCharIndex];
    state.inputs[state.currentWordIndex] += key;
    state.currentCharIndex += 1;
    state.stats.totalTypedChars += 1;

    if (key === expectedChar) {
        state.stats.correctChars += 1;
        playSound('correct');
    } else {
        state.stats.incorrectChars += 1;
        playSound('incorrect');
    }

    if (
        ['words', 'quote', 'custom'].includes(state.mode) &&
        state.currentWordIndex === state.words.length - 1 &&
        state.currentCharIndex === currentWord.length
    ) {
        finishSession();
        return;
    }

    updateLiveStats();
    renderWords();
}

function submitCurrentWord() {
    if (state.currentCharIndex === 0) return;

    state.submitted[state.currentWordIndex] = true;
    state.currentWordIndex += 1;
    state.currentCharIndex = 0;
    playSound('action');

    if (['words', 'quote', 'custom'].includes(state.mode) && state.currentWordIndex >= state.words.length) {
        finishSession();
        return;
    }

    if (state.currentWordIndex >= state.words.length) {
        state.currentWordIndex = state.words.length - 1;
    }

    renderWords();
}

function handleBackspace() {
    if (state.currentCharIndex === 0) {
        moveToPreviousWord();
        return;
    }

    const currentWord = state.words[state.currentWordIndex];
    const typedWord = state.inputs[state.currentWordIndex];
    const removeIndex = typedWord.length - 1;
    const removedChar = typedWord[removeIndex];
    const expectedChar = currentWord[removeIndex];

    state.inputs[state.currentWordIndex] = typedWord.slice(0, -1);
    state.currentCharIndex -= 1;
    updateStatsForRemoval(removedChar, expectedChar);
    updateLiveStats();
    renderWords();
}

function buildSummary() {
    let correctWords = 0;
    let wrongWords = 0;
    let correctChars = 0;
    let incorrectChars = 0;
    let missedChars = 0;

    state.words.forEach((word, index) => {
        const typedWord = state.inputs[index] || '';
        const isSubmitted = state.submitted[index];
        const includeWord = isSubmitted || typedWord.length > 0;
        if (!includeWord) return;

        let wordHasError = false;

        for (let charIndex = 0; charIndex < word.length; charIndex += 1) {
            const typedChar = typedWord[charIndex];
            if (typedChar === undefined) {
                if (isSubmitted) {
                    missedChars += 1;
                    wordHasError = true;
                }
                continue;
            }

            if (typedChar === word[charIndex]) {
                correctChars += 1;
            } else {
                incorrectChars += 1;
                wordHasError = true;
            }
        }

        if (typedWord.length !== word.length) {
            wordHasError = true;
        }

        if (wordHasError) {
            wrongWords += 1;
        } else {
            correctWords += 1;
        }
    });

    return { correctWords, wrongWords, correctChars, incorrectChars, missedChars };
}

function calculateConsistency() {
    if (state.history.wpm.length <= 1) return 100;

    const average = state.history.wpm.reduce((sum, value) => sum + value, 0) / state.history.wpm.length;
    if (!average) return 0;

    const variance = state.history.wpm.reduce((sum, value) => {
        return sum + ((value - average) * (value - average));
    }, 0) / state.history.wpm.length;

    const deviation = Math.sqrt(variance);
    return Math.max(0, Math.round(100 - ((deviation / average) * 100)));
}

function renderChart() {
    const context = document.getElementById('performance-chart').getContext('2d');
    if (chart) chart.destroy();

    chart = new Chart(context, {
        type: 'line',
        data: {
            labels: state.history.labels,
            datasets: [
                {
                    label: 'wpm',
                    data: state.history.wpm,
                    borderColor: '#71b8ff',
                    backgroundColor: 'rgba(113, 184, 255, 0.16)',
                    fill: true,
                    pointRadius: 0,
                    tension: 0.3,
                    borderWidth: 2
                },
                {
                    label: 'raw',
                    data: state.history.raw,
                    borderColor: '#63e6c6',
                    backgroundColor: 'transparent',
                    fill: false,
                    pointRadius: 0,
                    tension: 0.3,
                    borderWidth: 2,
                    borderDash: [6, 5]
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    labels: {
                        color: '#dbe5f8',
                        usePointStyle: true
                    }
                }
            },
            scales: {
                x: {
                    ticks: { color: '#97a4c0' },
                    grid: { color: 'rgba(151, 164, 192, 0.12)' }
                },
                y: {
                    beginAtZero: true,
                    ticks: { color: '#97a4c0' },
                    grid: { color: 'rgba(151, 164, 192, 0.12)' }
                }
            }
        }
    });
}

function finishSession() {
    if (state.isFinished) return;

    state.isFinished = true;
    clearInterval(state.timerInterval);
    markSnapshot();
    updateLiveStats();

    const elapsedMilliseconds = state.startTime ? (Date.now() - state.startTime) : 0;
    const elapsedMinutes = Math.max(elapsedMilliseconds / 60000, 1 / 60);
    const elapsedSeconds = Math.max(1, Math.round(elapsedMilliseconds / 1000));
    const summary = buildSummary();
    const accuracyBase = summary.correctChars + summary.incorrectChars + summary.missedChars;
    const accuracy = accuracyBase
        ? Math.round((summary.correctChars / accuracyBase) * 100)
        : 100;
    const raw = Math.round((state.stats.totalTypedChars / 5) / elapsedMinutes);
    const wpm = Math.round((summary.correctChars / 5) / elapsedMinutes);
    const consistency = calculateConsistency();
    const errorRate = Math.max(0, 100 - accuracy);

    document.getElementById('final-wpm').textContent = String(wpm);
    document.getElementById('res-raw').textContent = String(raw);
    document.getElementById('final-acc').textContent = `${accuracy}%`;
    document.getElementById('res-consistency').textContent = `${consistency}%`;
    document.getElementById('res-mode').textContent = state.mode;
    document.getElementById('res-time').textContent = getModeSummary();
    document.getElementById('res-duration').textContent = `${elapsedSeconds}s`;
    document.getElementById('res-words').textContent = `${summary.correctWords} / ${summary.wrongWords}`;
    document.getElementById('res-chars').textContent = `${summary.correctChars} / ${summary.incorrectChars} / ${summary.missedChars}`;
    document.getElementById('res-error-rate').textContent = `${errorRate}%`;
    document.getElementById('res-total-typed').textContent = String(state.stats.totalTypedChars);

    renderWords();
    renderChart();
    elements.gameView.classList.add('hidden');
    elements.resultView.classList.remove('hidden');
}

async function restart() {
    clearInterval(state.timerInterval);
    state.customText = elements.customText.value.trim() || state.customText || DEFAULT_CUSTOM_TEXT;

    updateOptionVisibility();
    updateHeadings();

    elements.promptLabel.textContent = 'loading prompt...';
    elements.progressLabel.textContent = 'preparing session...';

    state.words = await buildPromptWords();
    resetRuntimeState();

    elements.timerDisplay.textContent = state.mode === 'time' ? String(state.selectedTime) : state.mode === 'zen' ? 'zen' : 'ready';
    elements.gameView.classList.remove('hidden');
    elements.resultView.classList.add('hidden');
    elements.typingContainer.scrollTop = 0;
    updateLiveStats();
    updateHeadings();
    renderWords();
    focusTypingArea();
}

function shouldIgnoreGlobalTyping(event) {
    return event.target instanceof HTMLTextAreaElement;
}

async function handleKeydown(event) {
    if (shouldIgnoreGlobalTyping(event)) return;

    if (event.key === 'Tab') {
        event.preventDefault();
        await restart();
        return;
    }

    if (state.isFinished) return;

    if (event.key === 'Enter' && event.shiftKey && state.mode === 'zen') {
        event.preventDefault();
        finishSession();
        return;
    }

    if (event.key === 'Backspace') {
        event.preventDefault();
        handleBackspace();
        return;
    }

    if (event.key === ' ') {
        event.preventDefault();
        if (!state.startTime && state.currentCharIndex === 0) return;
        ensureStarted();
        submitCurrentWord();
        return;
    }

    if (event.key.length !== 1 || event.ctrlKey || event.metaKey || event.altKey) return;

    event.preventDefault();
    ensureStarted();
    typeCharacter(event.key);
}

function setMode(mode) {
    state.mode = mode;
    document.querySelectorAll('#modes .chip-button').forEach((button) => {
        button.classList.toggle('active', button.dataset.mode === mode);
    });
}

function bindModeControls() {
    document.querySelectorAll('#modes .chip-button').forEach((button) => {
        button.addEventListener('click', async () => {
            setMode(button.dataset.mode);
            await restart();
        });
    });

    document.querySelectorAll('#time-options .chip-button').forEach((button) => {
        button.addEventListener('click', async () => {
            document.querySelectorAll('#time-options .chip-button').forEach((item) => item.classList.remove('active'));
            button.classList.add('active');
            state.selectedTime = Number(button.dataset.value);
            await restart();
        });
    });

    document.querySelectorAll('#word-options .chip-button').forEach((button) => {
        button.addEventListener('click', async () => {
            document.querySelectorAll('#word-options .chip-button').forEach((item) => item.classList.remove('active'));
            button.classList.add('active');
            state.selectedWords = Number(button.dataset.value);
            await restart();
        });
    });

    document.querySelectorAll('#quote-options .chip-button').forEach((button) => {
        button.addEventListener('click', async () => {
            document.querySelectorAll('#quote-options .chip-button').forEach((item) => item.classList.remove('active'));
            button.classList.add('active');
            state.selectedQuoteWords = Number(button.dataset.value);
            await restart();
        });
    });
}

function bindToggles() {
    elements.togglePunctuation.addEventListener('click', async () => {
        state.punctuation = !state.punctuation;
        elements.togglePunctuation.classList.toggle('active', state.punctuation);
        elements.togglePunctuation.setAttribute('aria-pressed', String(state.punctuation));
        await restart();
    });

    elements.toggleNumbers.addEventListener('click', async () => {
        state.numbers = !state.numbers;
        elements.toggleNumbers.classList.toggle('active', state.numbers);
        elements.toggleNumbers.setAttribute('aria-pressed', String(state.numbers));
        await restart();
    });

    elements.toggleSound.addEventListener('click', () => {
        state.soundEnabled = !state.soundEnabled;
        updateSoundToggle();
        if (state.soundEnabled) playSound('action');
    });
}

function bindActions() {
    elements.restartBtn.addEventListener('click', restart);
    elements.playAgainBtn.addEventListener('click', restart);
    elements.typingContainer.addEventListener('click', focusTypingArea);
    elements.customText.addEventListener('input', () => {
        state.customText = elements.customText.value.trim() || DEFAULT_CUSTOM_TEXT;
    });
    elements.applyCustomBtn.addEventListener('click', async () => {
        state.customText = elements.customText.value.trim() || DEFAULT_CUSTOM_TEXT;
        elements.customText.blur();
        elements.applyCustomBtn.blur();
        await restart();
    });
}

function initialize() {
    elements.customText.value = state.customText;
    updateSoundToggle();
    bindModeControls();
    bindToggles();
    bindActions();
    window.addEventListener('keydown', handleKeydown);
    restart();
}

initialize();
