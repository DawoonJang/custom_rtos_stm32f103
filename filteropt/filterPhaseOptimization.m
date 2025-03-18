function phaseError = phaseOptimization(params, sample_rate, target_frequency)
    cutoff_freq = params.CUTOFF_FREQ;
    order = 4;

    [b, a] = butter(order, cutoff_freq / (sample_rate / 2), 'high');

    t = (0:63) / sample_rate;
    signal = sin(2 * pi * 1500 * t) + sin(2 * pi * target_frequency * t) + 0.5 * sin(2 * pi * 60 * t);
    
    filtered_signal = filter(b, a, signal);

    fft_filtered = fft(filtered_signal);
    phase_filtered = angle(fft_filtered);
    magnitude_filtered = abs(fft_filtered);  % 진폭 응답 계산

    % 주파수 벡터 생성
    freqs = (0:length(fft_filtered)-1) * (sample_rate / length(fft_filtered));

    % 목표 주파수에서의 위상 값 찾기
    targetPhase = 0;
    [~, freqIdx] = min(abs(freqs - target_frequency));
    phaseError = abs(phase_filtered(freqIdx) - targetPhase);

    % 추가 조건: 필터링된 신호의 진폭도 고려
    min_magnitude_threshold = 0.1;  % 최소 진폭 기준 설정
    if magnitude_filtered(freqIdx) < min_magnitude_threshold
        phaseError = phaseError + 10;  % 진폭이 너무 작으면 패널티 추가
    end
end

function amplitudeError = amplitudeOptimization(params, sample_rate, target_frequency)
    cutoff_freq = params.CUTOFF_FREQ;
    order = 4;
    
    % Butterworth 필터 설계 (고역통과 필터)
    [b, a] = butter(order, cutoff_freq / (sample_rate / 2), 'high');
    
    % 신호 생성
    t = (0:63) / sample_rate;
    signal = sin(2 * pi * 1500 * t) + sin(2 * pi * target_frequency * t) + 0.5 * sin(2 * pi * 60 * t);
    
    % 신호 필터링
    filtered_signal = filter(b, a, signal);
    
    % FFT 계산
    fft_filtered = fft(filtered_signal);
    magnitude_filtered = abs(fft_filtered);  % 진폭 계산
    
    % 주파수 벡터 생성
    freqs = (0:length(fft_filtered)-1) * (sample_rate / length(fft_filtered));
    
    % 목표 주파수에서의 진폭 값 찾기
    [~, freqIdx] = min(abs(freqs - target_frequency));
    
    % 최대 진폭을 최적화하는 경우, 전체 신호에서 최대 진폭을 찾아오도록 수정
    maxAmplitude = max(magnitude_filtered);  % 최대 진폭
    amplitudeError = abs(magnitude_filtered(freqIdx) - maxAmplitude);  % 목표 진폭 차이 계산
end
clc; clear;
% 샘플링 주파수
SAMPLE_RATE = 4000;
TARGET_FREQUENCY = 1200;

% 베이지안 최적화 함수 정의
objectiveFunction = @(params) phaseOptimization(params, SAMPLE_RATE, TARGET_FREQUENCY);
% objectiveFunction = @(params) amplitudeOptimization(params, SAMPLE_RATE, TARGET_FREQUENCY);

paramSpace = [
    optimizableVariable('CUTOFF_FREQ', [100, 2000])
];

% 베이지안 최적화 실행
results = bayesopt(objectiveFunction, paramSpace, 'MaxObjectiveEvaluations', 30);

bestCutoff = results.XAtMinObjective.CUTOFF_FREQ;
[b_opt, a_opt] = butter(4, bestCutoff / (SAMPLE_RATE / 2));

set(gcf, 'Color', 'k');  % 'k'는 검은색을 의미합니다.
disp('최적화된 차단 주파수:');
disp(bestCutoff);
disp('최적화된 필터 계수 b:');
disp(b_opt);
disp('최적화된 필터 계수 a:');
disp(a_opt);