# 단계별 IOCP 실습
* C++20을 사용하여 제작
* 최대한 예시코드를 보지 않고 만드는 것을 의식
* [학습링크(github)](https://github.com/jacking75/edu_cpp_IOCP)


## 완료한 단계
* 1 단계. 가장 간단한 IOCP 서버. Echo 서버 코드 만들기
    - 이 단계만큼은 따라서 코딩함(네이밍의 차이 있음)
    - 영상에서 설명한 것처럼 IOCP의 개념과 API를 이해하는 것이 목적
    - [참고자료1(소켓 강좌)](https://gpgstudy.com/gpgiki/소켓%20강좌)
    - [참고자료2(MS공식문서)](https://learn.microsoft.com/ko-kr/windows/win32/fileio/i-o-completion-ports)
    - [참고자료3(IOCP 기본 구조 이해)](https://www.slideshare.net/namhyeonuk90/iocp)
    - [참고자료4(IOCP 모델 이론)](https://marmelo12.tistory.com/265)
* 2 단계. OverlappedEx 구조체에 있는 char m_szBuf[ MAX_SOCKBUF ]를 stClientInfo 구조체로 이동 및 코드 분리하기
    - stClientInfo에 readBuf, sendBuf 를 추가하여 구조 및 코드 변경
* 3 단계. 애플리케이션과 네트워크 코드 분리하기
    - has-a를 사용하여 네트워크와 서버 로직 분리 구현
    - std::function을 사용하여 Connect, Disconnect, Receive 콜백을 구현