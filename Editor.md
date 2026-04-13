# 에디터 구조
ShellEngine의 에디터는 엔진이 에디터 모드로 부팅된 상태입니다.</br>
이런 구조로 인해 월드, 카메라, 렌더러, 입력, GC, 스레드 모델을 대부분 공유하면서도 에디터 전용 타입만 별도로 끼워 넣을 수 있습니다.

- ```Project```
  -  새 프로젝트 생성, 프로젝트 열기, 월드 저장/불러오기, 빌드, 모듈 리로드
- ```EditorWorld```
  -  일반 game::World를 상속한 편집용 월드
  -  선택 상태, 에디터 카메라, 피킹 카메라, 뷰포트 렌더 텍스처, 그리드/축, EditorUI 컴포넌트를 붙인 에디터용 게임오브젝트를 관리
  -  에디터 전용 편집 상태를 분리해서 기록하고, 정지 시에는 world point를 되돌려 플레이 이전 상태를 복원
- ```EditorRenderer```
  - 일반 렌더러를 그대로 쓰지 않고 에디터 전용 패스를 조합한 ```ScriptableRenderer```
  - 피킹, 아웃라인 pre/post pass, 일반 Opaque/Transparent, UI pass, ImGui pass를 카메라별로 필터링
- ```EditorUI```
  - Viewport, Hierarchy, Inspector, Project 패널과 각종 툴 UI를 생성하고, 단축키와 메뉴를 라우팅
- ```AssetDatabase```
  - 파일 경로와 UUID 매핑 유지
  - 원본 파일, .meta, 캐시 .asset 사이 동기화
  - 에셋 타입별 로더 선택 및 의존 순서 조절

# 엔진 생명주기
에디터 실행은 src/main.cpp에서 ```EngineInit::Start()```를 호출하는 순간 시작됩니다. 그 후 에디터와 게임 모드의 실행 분기점은 ```#if SH_EDITOR``` 블록에서 나눠집니다.

그 다음으로 이루어지는 과정은 다음과 같습니다.

1. ```AssetResolverRegistry```에 에디터용 resolver를 등록
   -  resolver: 메모리에 안 올라온 에셋을 불러올 때 어떻게 처리 할 것인지 정의
3. ```Project``` 인스턴스를 생성
4. ```EditorWorld``` 팩토리를 등록
5. 기본 ```EditorWorld```를 하나 만들고 ```GameManager```에 메인 월드로 로드
6. ```EditorResource```와 에디터 렌더링 리소스를 초기화
7. ```GameManager```에서 아래 이미지의 루프를 실행

<img width="379" height="731" alt="생명주기 drawio" src="https://github.com/user-attachments/assets/2a2561d9-069d-4fa8-99ad-73f7b842de66" />

OnEnable과 OnDisable은 객체가 활성화/비활성화 되는 즉시 실행 됩니다.

> [!NOTE]
> 루프중에 생성된 객체는 다음 루프가 시작 된 후부터 함수들이 실행됩니다.

# 플레이
![image](https://github.com/user-attachments/assets/c5749b0d-bb95-4d84-b44f-2f0f1822f8bb)

에디터에서 게임 플레이 환경을 미리 볼 수 있습니다.

플레이 모드에 들어가면 world의 메인 카메라를 기준으로 화면이 전환되며, 컴포넌트들이 생명주기대로 실행됩니다.

만약 에디터 상에서 컴포넌트를 작동 시키고 싶다면 컴포넌트의 canPlayInEditor변수를 생성자에서 true로 설정 하면 됩니다.

# 빌드
<img width="513" height="510" alt="image" src="https://github.com/user-attachments/assets/bf391678-6d1c-4d5b-a6c0-c32c68974128" /></br>
<img width="246" height="232" alt="image" src="https://github.com/user-attachments/assets/748085fc-9f1e-4da0-bb29-cc60003d34a5" />

프로젝트 설정에서 월드를 지정하고 빌드 버튼을 누르면 해당 월드와 연결된 모든 에셋이 에셋번들과 게임 설정 파일이 내보내집니다.</br>
이 상태에서 ShellGame.exe를 실행하면 게임이 실행 됩니다.

<img width="802" height="557" alt="image" src="https://github.com/user-attachments/assets/fa80aa66-5bca-41d8-b641-6d743fd6d488" />

Tools - Bundle Viewer를 통해 에셋 번들에 어떤 에셋이 들어갔는지 확인 할 수도 있습니다.

# 조작 가이드 영상
[![Video Label](http://img.youtube.com/vi/SEiktv0WtOM/0.jpg)](https://youtu.be/SEiktv0WtOM)</br>
https://youtu.be/SEiktv0WtOM

## 조작 방법
시점 회전: alt + 마우스 좌클릭</br>
시점 확대/축소: 마우스 휠 굴리기</br>
시점 이동: 마우스 휠 누른채 마우스 이동</br>

물체 이동: 물체 선택 후 G</br>
물체 회전: 물체 선택 후 R</br>
물체 크기: 물체 선택 후 S</br>
특정축 조작: G/R/S 후 X/Y/Z</br>

물체 복사: 물체 선택 후 Ctrl + D
