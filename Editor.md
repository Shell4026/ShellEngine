# 개요
효율적인 게임 제작을 위해 필요한 요소인 에디터를 구현했습니다. </br>
에디터는 Unity 엔진의 UI와 비슷하며 조작법은 Blender와 유사 합니다. </br>
더욱 많은 기능을 개발중입니다.</br>

# 엔진 생명주기
<img width="379" height="731" alt="생명주기 drawio" src="https://github.com/user-attachments/assets/2a2561d9-069d-4fa8-99ad-73f7b842de66" />

OnEnable과 OnDisable은 객체가 활성화/비활성화 되는 즉시 실행 됩니다.

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
# 조작 영상(구)
[![Video Label](http://img.youtube.com/vi/D48w8sa7JFQ/0.jpg)](https://youtu.be/D48w8sa7JFQ?si=d-AvSkDpJbcnExlB)

# 조작 방법
시점 회전: alt + 마우스 좌클릭</br>
시점 확대/축소: 마우스 휠 굴리기</br>
시점 이동: 마우스 휠 누른채 마우스 이동</br>

물체 이동: 물체 선택 후 G</br>
물체 회전: 물체 선택 후 R</br>
물체 크기: 물체 선택 후 S</br>
특정축 조작: G/R/S 후 X/Y/Z</br>

물체 복사: 물체 선택 후 Ctrl + D
