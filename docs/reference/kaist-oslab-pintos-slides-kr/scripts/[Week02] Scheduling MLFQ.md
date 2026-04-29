# [Week02] Scheduling MLFQ

Source: https://youtu.be/57r9OCN1EfA?si=z-hYixEB1a_zG6-d

## Intro

다음 주제는 multi-level feedback queue, 즉 MLFQ입니다.

shortest-job-first scheduling이나 round-robin scheduling에서는 모든 것이 고정되어 있습니다. system 안에는 job queue가 하나만 있습니다. 이제 우리는 과거로부터 학습해 미래를 예측하고, job의 behavior(행동)에 따라 priority를 조정하는 새로운 scheduler를 논의할 것입니다.

목표는 CPU time을 많이 필요로 하지 않는 process에게 priority를 주고, CPU time을 많이 필요로 하는 process의 priority를 낮추는 것입니다. 이를 통해 job의 CPU time usage behavior를 미리 알지 못해도 response time을 최소화합니다.

## MLFQ 동작 방식

multi-level feedback queue는 여러 개의 distinct queue(구별되는 큐)를 가집니다. 예를 들어 30개에서 64개 정도의 queue를 가질 수 있습니다. 각 queue에는 서로 다른 priority level이 할당됩니다. ready 상태의 job은 하나의 queue에 들어가며, job은 여러 queue 중 하나에 위치할 수 있습니다.

각 queue는 round-robin scheduling algorithm으로 schedule됩니다.

MLFQ에서 기억해야 할 rule은 다음과 같습니다.

- priority A가 priority B보다 크면 A가 실행됩니다.
- 두 process의 priority가 같으면 round-robin 방식으로 실행됩니다.
- 같은 queue에 있는 job들은 같은 priority를 가집니다.

MLFQ는 관찰된 behavior를 기준으로 job의 priority를 바꿉니다. 예를 들어 어떤 job이 I/O를 기다리기 위해 CPU를 반복해서 양보한다면, 그 job의 priority는 높게 유지됩니다. 반대로 어떤 job이 오랫동안 CPU를 집중적으로 사용한다면, 그 job의 priority는 낮아집니다.

예를 들어 system에 8개의 priority queue와 4개의 job이 있다고 합시다. 가장 높은 priority는 Q8이고 가장 낮은 priority는 Q1입니다. job A와 B는 가장 높은 priority queue인 Q8에 있고, job C는 Q4에 있으며, job D는 Q1에 있습니다. scheduler는 비어 있지 않은 queue 중 가장 높은 priority queue의 job을 먼저 실행합니다.

## Priority 조정

priority adjustment algorithm의 rule은 다음과 같습니다.

- job이 처음 system에 들어오면 가장 높은 priority에 배치됩니다.
- job이 실행 중에 전체 time slice를 모두 사용하면 priority가 낮아지고, 다음 priority level로 내려갑니다.
- job이 time slice가 끝나기 전에 CPU를 양보하면 같은 priority level에 머무릅니다.

single long-running job 예시를 생각해 봅시다. priority queue가 3개 있고 time slice가 10ms라고 가정합니다. job이 도착하면 가장 높은 priority queue인 Q2에 삽입됩니다.

job은 처음 10ms 동안 실행됩니다. 전체 time slice를 모두 사용했으므로 다음 priority level로 내려갑니다. 그런 다음 다시 10ms 동안 실행됩니다. 이 time slice도 모두 사용하면 가장 낮은 priority level인 Q0으로 내려갑니다. 이후에는 그곳에 머무르며 가장 낮은 level에서 계속 실행됩니다.

## Time Quantum

또 다른 이슈는 time quantum입니다. CPU utilization의 변화에 반응하기 위해 MLFQ는 queue의 priority에 따라 서로 다른 time quantum length를 줄 수 있습니다.

high-priority queue에는 더 짧은 time slice를 사용합니다. lower-priority queue에는 더 긴 time quantum을 사용할 수 있습니다.

queue가 3개 있는 system을 보면, 가장 높은 priority queue는 매우 짧은 time slice length를 가지지만 낮은 priority queue는 더 긴 time slice length를 가집니다.

## Solaris와 FreeBSD

Solaris는 time-sharing scheduling class를 사용해 multi-level feedback을 구현합니다. Solaris는 60개의 queue를 가지고, queue priority에 따라 time slice length를 점진적으로 증가시킵니다. 가장 높은 priority queue는 20ms time slice를 가지는 반면, 가장 낮은 priority queue는 수백 ms 정도의 time slice를 가집니다. job의 priority는 대략 1초마다 boost됩니다.

FreeBSD scheduler는 MLFQ를 구현하는 또 다른 방식입니다. FreeBSD 구현이 흥미로운 이유는 실제 queue 없이 MLFQ를 구현하기 때문입니다. 대신 equation(공식)을 사용합니다. process가 CPU를 얼마나 사용했는지를 바탕으로 process priority를 계산하며, 이는 MLFQ와 같은 아이디어입니다. 또한 decay를 통해 priority를 boost하고, 다른 process에게 CPU를 양보하려는 user의 의도도 반영합니다. 효율성을 위해 queue도 사용합니다.

우리 Pintos project에서는 FreeBSD scheduler를 자세히 구현할 것입니다. 자세한 내용은 수업의 뒤쪽 부분에서 다룹니다.

## Summary

MLFQ는 다음 rule들의 집합으로 요약할 수 있습니다.

- process의 priority가 더 높으면 그 process가 실행됩니다.
- 두 process의 priority가 같으면 round-robin 방식으로 실행됩니다.
- job이 system에 들어오면 가장 높은 priority level에 배치됩니다.
- job이 주어진 level에서 자신의 time quantum을 모두 사용하면 다음 priority level로 내려갑니다.
- 일정 시간이 지나면 system 안의 모든 job을 가장 높은 queue로 이동시킵니다.

MLFQ의 장점은 process의 CPU behavior에 대한 prior knowledge가 필요하지 않다는 것입니다.
