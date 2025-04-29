# 개요
효율적인 게임 제작을 위해 필요한 요소인 에디터를 구현했습니다. </br>
에디터는 Unity 엔진의 UI와 비슷하며 조작법은 Blender와 유사 합니다. </br>
더욱 많은 기능을 개발중입니다.</br>

# 조작 영상
[![Video Label](http://img.youtube.com/vi/D48w8sa7JFQ/0.jpg)](https://youtu.be/D48w8sa7JFQ?si=d-AvSkDpJbcnExlB)

(자막에 설명 있음)

# 조작 방법
시점 회전: alt + 마우스 좌클릭</br>
시점 확대/축소: 마우스 휠 굴리기</br>
시점 이동: 마우스 휠 누른채 마우스 이동</br>

물체 이동: 물체 선택 후 G</br>
물체 회전: 물체 선택 후 R</br>
물체 크기: 물체 선택 후 S</br>
특정축 조작: G/R/S 후 X/Y/Z</br>

물체 복사: 물체 선택 후 Ctrl + D

# 플레이
![image](https://github.com/user-attachments/assets/c5749b0d-bb95-4d84-b44f-2f0f1822f8bb)

에디터에서 게임 플레이 환경을 미리 볼 수 있습니다.

플레이 모드에 들어가면 world의 메인 카메라를 기준으로 화면이 전환되며, 컴포넌트들이 생명주기대로 실행됩니다.

만약 에디터 상에서 컴포넌트를 작동 시키고 싶다면 컴포넌트의 canPlayInEditor변수를 생성자에서 true로 설정 하면 됩니다.

# 엔진 생명주기
![생명주기 drawio](https://github.com/user-attachments/assets/7c99b8e1-58fd-4d2b-91da-89c0d45fe67a)

