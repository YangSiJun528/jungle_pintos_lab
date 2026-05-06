---
marp: true
theme: default
paginate: false
size: 16:9
backgroundColor: #f8fafc
style: |
  section {
    padding: 78px 92px;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    color: #111827;
  }
  h1 {
    margin: 0;
    font-size: 32px;
    letter-spacing: 0;
  }
  .muted {
    color: #64748b;
  }
  .wrap {
    margin-top: 48px;
    display: grid;
    grid-template-columns: 1.1fr 1fr;
    gap: 36px;
    align-items: center;
  }
  .big {
    font-size: 104px;
    line-height: 0.9;
    font-weight: 850;
    letter-spacing: 0;
  }
  .rate {
    margin-top: 16px;
    font-size: 38px;
    font-weight: 800;
    color: #15803d;
  }
  .bar {
    margin-top: 28px;
    height: 16px;
    border-radius: 999px;
    overflow: hidden;
    background: #fee2e2;
  }
  .bar-pass {
    width: 96.9%;
    height: 100%;
    background: #16a34a;
  }
  .stats {
    display: grid;
    gap: 13px;
  }
  .stat {
    display: flex;
    justify-content: space-between;
    align-items: center;
    border: 1px solid #e2e8f0;
    border-radius: 8px;
    background: #fff;
    padding: 16px 18px;
    font-size: 21px;
  }
  .stat strong {
    font-size: 25px;
  }
  .ok {
    color: #15803d;
  }
  .fail {
    color: #dc2626;
  }
  .foot {
    margin-top: 36px;
    font-size: 17px;
    color: #64748b;
  }
---

# Project 2 Userprog Progress
<div class="muted">기본 check + stress/extra 실패 항목 포함</div>

<div class="wrap">
  <div>
    <div class="big">94 / 97</div>
    <div class="rate">96.9% PASS</div>
    <div class="bar"><div class="bar-pass"></div></div>
  </div>

  <div class="stats">
    <div class="stat"><span>기본 통과 항목</span><strong class="ok">94 / 94</strong></div>
    <div class="stat"><span>multi-oom</span><strong class="fail">0 / 1</strong></div>
    <div class="stat"><span>dup2 extra</span><strong class="fail">0 / 2</strong></div>
  </div>
</div>

<div class="foot">Fail: multi-oom, dup2-simple, dup2-complex</div>
