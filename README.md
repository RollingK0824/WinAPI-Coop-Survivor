# WinAPI Co-op Vampire Survivors (가제)
> **C++20 & Direct2D 기반 최대 4인 온라인 협동(Co-op) 액션 로그라이크 게임 프로젝트**
---
## 프로젝트 개요 (Overview)
- Windows API 및 자체 제작 프레임워크([D2DFramework](https://github.com/RollingK0824/D2DFramework))를 기반으로 개발하는 **온라인 멀티플레이 Vampire Survivors 라이크** 게임  
- WinSock2 소켓 통신을 통해 최대 **4명의 플레이어**가 한 방에 접속하여 몬스터 웨이브를 함께 막아내며 생존하는 것이 목표
---
##  주요 개발 목표 (Key Goals)
- **최대 4인 멀티플레이 (4-Player Network Co-op)**
  - WinSock 2.2 기반 비동기 소켓 통신을 이용한 호스트-클라이언트 멀티플레이
  - 플레이어 동기화 및 몬스터/투사체 상태 동기화
- **대규모 몬스터 스웜 & 렌더링 최적화**
  - 오브젝트 풀링(Object Pooling)을 통한 대량의 몬스터/투사체 관리
  - 프레임 드랍 최소화를 위한 스폰 및 렌더링 최적화
- **협동 로그라이크 요소**
  - 다양한 무기 및 스킬 조합
  - 팀원 간 시너지 효과 및 사망한 팀원 부활/지원 시스템
---
## 기술 스택 (Tech Stack)
- **Language**: C++20
- **Framework**: Custom D2DFramework (Win32 API)
- **Graphics**: Direct2D , DirectWrite, WIC
- **Network**: WinSock 2.2 (TCP/UDP Socket)
- **Physics**: Box2D 3.0
- **GUI / Debug**: Dear ImGui
## 조작법 (Controls)
- **이동**: `W`, `A`, `S`, `D` (8방향 이동)
- **공격**: 범위 내 적 자동 타겟팅 및 발사 (자동 공격)
- **디버그 메뉴**: `F3` (Dear ImGui 디버그 오버레이)
---
## 개발 진행 현황 (Progress)
- [ ] 리포지토리 세팅 및 D2DFramework 연동
- [ ] 소켓 통신 4인 접속 테스트 (Lobby & Room)
- [ ] 멀티플레이어 캐릭터 이동 동기화
- [ ] 몬스터 동기화 및 대규모 스폰 테스트
- [ ] 무기/스킬 및 레벨업 UI 구현
