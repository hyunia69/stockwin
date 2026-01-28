# 한국경제TV ALLAT ARS 결제 시나리오 Flowchart

## 1. 전체 시나리오 흐름도 (Main Flow)

```mermaid
flowchart TD
    START([전화 연결]) --> INTRO[1. 인사말<br/>wownet_intro]
    
    INTRO --> TYPE_CHECK{시나리오 타입}
    
    TYPE_CHECK -->|ARS| ARS_INPUT[전화번호 입력]
    TYPE_CHECK -->|SMS| SMS_INPUT[주문번호 입력]
    TYPE_CHECK -->|CID| CID_AUTO[Caller ID 자동조회]
    TYPE_CHECK -->|CIP| CIP_CHECK{휴대폰 번호?}
    
    CIP_CHECK -->|Yes| CID_AUTO
    CIP_CHECK -->|No| CIP_INPUT[전화번호 입력]
    
    ARS_INPUT --> ORDER_QUERY[주문정보 조회]
    SMS_INPUT --> ORDER_QUERY
    CID_AUTO --> ORDER_QUERY
    CIP_INPUT --> ORDER_QUERY
    
    ORDER_QUERY --> ORDER_RESULT{조회 결과}
    
    ORDER_RESULT -->|시스템 장애| SYS_ERROR[시스템 장애 안내]
    ORDER_RESULT -->|주문 없음| NO_ORDER[주문 없음 안내]
    ORDER_RESULT -->|와우캐시 결제완료| WOWCASH[와우캐시 결제완료 안내]
    ORDER_RESULT -->|정상| CONSENT[2. 동의서 안내]
    
    SYS_ERROR --> EXIT_SVC
    NO_ORDER --> EXIT_SVC
    WOWCASH --> EXIT_SVC
    
    CONSENT --> CONSENT_CONFIRM{동의 여부}
    CONSENT_CONFIRM -->|1번 동의| PAYMENT_INFO[3. 결제금액/해지조건 안내]
    CONSENT_CONFIRM -->|2번 미동의| TYPE_CHECK
    
    PAYMENT_INFO --> PAYMENT_CONFIRM{결제 동의}
    PAYMENT_CONFIRM -->|1번 동의| CARD_INPUT[4. 카드번호 입력]
    PAYMENT_CONFIRM -->|2번 취소| TYPE_CHECK
    
    CARD_INPUT --> CARD_CONFIRM{확인}
    CARD_CONFIRM -->|1번 맞음| EXPIRE_INPUT[5. 유효기간 입력]
    CARD_CONFIRM -->|2번 틀림| CARD_INPUT
    
    EXPIRE_INPUT --> EXPIRE_CONFIRM{확인}
    EXPIRE_CONFIRM -->|1번 맞음| AMOUNT_CHECK{금액 체크}
    EXPIRE_CONFIRM -->|2번 틀림| EXPIRE_INPUT
    
    AMOUNT_CHECK -->|5만원 미만| PAYMENT_PROCESS[7. 결제 처리]
    AMOUNT_CHECK -->|5만원 이상| INSTALL_INPUT[6. 할부개월 선택]
    
    INSTALL_INPUT --> INSTALL_CONFIRM{확인}
    INSTALL_CONFIRM -->|1번 맞음| PAYMENT_PROCESS
    INSTALL_CONFIRM -->|2번 틀림| INSTALL_INPUT
    
    PAYMENT_PROCESS --> PAY_RESULT{결제 결과}
    PAY_RESULT -->|성공| PAY_SUCCESS[결제 완료 안내]
    PAY_RESULT -->|실패| PAY_FAIL[결제 실패 안내]
    
    PAY_SUCCESS --> EXIT_SVC[8. 종료]
    PAY_FAIL --> EXIT_SVC
    
    EXIT_SVC --> END_CALL([통화 종료])
```

---

## 2. 상세 Flowchart (멘트 포함)

### 2-1. 인사말 및 고객식별

```mermaid
flowchart TD
    subgraph GREETING["1. 인사말"]
        G1["audio/shop_intro/wownet_intro<br/>━━━━━━━━━━━━━━━━━<br/>와우넷 인사말"]
    end
    
    subgraph ARS_MODE["ARS 모드: 전화번호 입력"]
        A1["ment/_common/common_audio/input_telnum_start<br/>━━━━━━━━━━━━━━━━━<br/>전화번호를 입력해 주세요"]
        A2["[TTS] 고객님께서 누르신 전화번호는<br/>0,1,0,1,2,3,4,5,6,7,8 번 입니다"]
        A3["ment/_common/common_audio/input_confirm<br/>━━━━━━━━━━━━━━━━━<br/>맞으면 1번, 틀리면 2번"]
        A1 --> A2 --> A3
    end
    
    subgraph SMS_MODE["SMS 모드: 주문번호 입력"]
        S1["audio/input_sms_start<br/>━━━━━━━━━━━━━━━━━<br/>SMS로 받은 주문번호 6자리 입력"]
        S2["audio/input_sms_msg + [TTS]<br/>━━━━━━━━━━━━━━━━━<br/>입력하신 주문번호는 X,X,X,X,X,X 번"]
        S3["ment/_common/common_audio/input_confirm<br/>━━━━━━━━━━━━━━━━━<br/>맞으면 1번, 틀리면 2번"]
        S1 --> S2 --> S3
    end
    
    subgraph CID_MODE["CID/CIP 모드"]
        C1["Caller ID 자동 획득"]
        C2{휴대폰 번호?}
        C3["전화번호 입력 요청"]
        C1 --> C2
        C2 -->|No| C3
    end
    
    GREETING --> ARS_MODE
    GREETING --> SMS_MODE
    GREETING --> CID_MODE
```

### 2-2. 주문정보 조회 및 동의서

```mermaid
flowchart TD
    subgraph ORDER_QUERY["주문정보 조회 결과"]
        direction TB
        Q1{조회 결과}
        
        Q1 -->|시스템 장애| E1["ment/ALLAT_WOWTV/Tcp_Error<br/>━━━━━━━━━━━━━━━━━<br/>TCP 통신 에러"]
        Q1 -->|주문 없음| E2["ment/_common/common_audio/no_order_msg<br/>━━━━━━━━━━━━━━━━━<br/>주문이 접수되지 않았습니다.<br/>상점으로 문의하여 주시기 바랍니다"]
        Q1 -->|와우캐시| E3["[TTS] {파트너명} 측에서는<br/>보유하고 계신 와우캐시로<br/>정상적으로 결제되었습니다"]
        Q1 -->|정상| CONSENT
    end
    
    subgraph CONSENT["2. 동의서 안내"]
        direction TB
        CON1["[TTS] 한국경제TV를 통해 제공되는<br/>각종 정보제공 및 기타 부가서비스를 위하여<br/>{파트너명}에게 고객님의 아이디, 필명,<br/>휴대폰전화번호, 가입기간을<br/>해당 서비스 종료일까지<br/>열람및 이용할 수 있도록<br/>정보제공되는것에 동의하셔야 합니다"]
        CON2["동의하시겠습니까?<br/>동의를 거부하실 권리가 있으며<br/>동의를 거부하실 경우<br/>해당 서비스를 이용하실 수 없습니다"]
        CON3["동의하시려면 1번을<br/>동의하지 않으시면 2번을 눌러주십시오"]
        CON1 --> CON2 --> CON3
    end
    
    CON3 --> CONFIRM{선택}
    CONFIRM -->|1번| NEXT[결제정보 안내로 이동]
    CONFIRM -->|2번| BACK[고객식별 재입력]
    
    E1 --> EXIT[종료]
    E2 --> EXIT
    E3 --> EXIT
```

### 2-3. 결제금액 및 해지조건 안내

```mermaid
flowchart TD
    subgraph PAYMENT_INFO["3. 결제금액 안내"]
        direction TB
        
        P_CHECK{할인 유형}
        
        P_CHECK -->|쿠폰+보너스캐시| P1["[TTS] {고객명} 고객님, {가맹점명}에서<br/>주문하신 {상품명}의 결제 금액은<br/>{고객명}님께서 보유하신<br/>{쿠폰명} 쿠폰과 보너스캐시 {금액}원이<br/>적용되어 최종 결제금액은 {결제금액}원입니다"]
        
        P_CHECK -->|쿠폰만| P2["[TTS] {고객명} 고객님, {가맹점명}에서<br/>주문하신 {상품명}의 결제 금액은<br/>{고객명}님께서 보유하신<br/>{쿠폰명} 쿠폰이 적용되어<br/>최종 결제금액은 {결제금액}원입니다"]
        
        P_CHECK -->|보너스캐시만| P3["[TTS] {고객명} 고객님, {가맹점명}에서<br/>주문하신 {상품명}의 결제 금액은<br/>{고객명}님께서 보유하신<br/>보너스캐시 {금액}원이 적용되어<br/>최종 결제금액은 {결제금액}원입니다"]
        
        P_CHECK -->|할인없음| P4["[TTS] {고객명} 고객님, {가맹점명}에서<br/>주문하신 {상품명}의<br/>결제하실 금액은 {결제금액}원입니다"]
    end
    
    P1 --> TERMS
    P2 --> TERMS
    P3 --> TERMS
    P4 --> TERMS
    
    subgraph TERMS["해지조건 안내"]
        direction TB
        
        T_CHECK{상품유형}
        
        T_CHECK -->|TABLET| T1["[TTS] 박스 개봉 후 제품 불량을<br/>제외하고는 교환 및 반품이 불가합니다"]
        
        T_CHECK -->|EDUCATION| T2["[TTS] 본 상품은 교육 상품으로<br/>결제 후 해지가 불가능합니다<br/>또한 무료로 제공되는 서비스는<br/>중도해지 및 일시정지 파일양도가 불가합니다"]
        
        T_CHECK -->|SERVICE| T3["[TTS] 서비스 중도해지 시<br/>해지일까지 이용요금과<br/>해지수수료 10%와<br/>제공받으신 사은품 정가가<br/>함께 차감됩니다"]
    end
    
    T1 --> INVEST
    T2 --> INVEST
    T3 --> EXPIRE_CHECK
    
    subgraph EXPIRE_CHECK["쿠폰/보너스캐시 소멸 안내 (SERVICE만)"]
        direction TB
        EX_CHECK{할인 적용?}
        
        EX_CHECK -->|쿠폰+보너스캐시| EX1["[TTS] 또한 자동결제 시 적용되는<br/>{쿠폰명} 쿠폰과 보너스캐시 {금액}원은<br/>소멸됨을 알려드립니다"]
        EX_CHECK -->|쿠폰만| EX2["[TTS] 또한 자동결제 시 적용되는<br/>{쿠폰명} 쿠폰이 소멸됨을 알려드립니다"]
        EX_CHECK -->|보너스캐시만| EX3["[TTS] 또한 자동결제 시 적용되는<br/>{금액}원 보너스캐시는<br/>소멸됨을 알려드립니다"]
        EX_CHECK -->|없음| EX4[다음 단계]
    end
    
    EX1 --> INVEST
    EX2 --> INVEST
    EX3 --> INVEST
    EX4 --> INVEST
    
    subgraph INVEST["투자 유의사항"]
        INV["[TTS] 또한 한국경제티비와 파트너는<br/>금융투자업자가 아닌 유사투자자문업자로<br/>개별적인 투자상담과 자금운영이 불가하며<br/>원금 손실이 발생할 수 있고<br/>그 손실은 투자자에게 귀속됩니다"]
    end
    
    INV --> FINAL_CONFIRM["[TTS] 동의 및 결제하시려면 1번을<br/>취소하시려면 2번을 눌러주세요"]
    
    FINAL_CONFIRM --> CHOICE{선택}
    CHOICE -->|1번| CARD[카드번호 입력으로]
    CHOICE -->|2번| RETRY[고객식별 재입력]
```

### 2-4. 카드정보 입력

```mermaid
flowchart TD
    subgraph CARD_INPUT["4. 카드번호 입력"]
        direction TB
        C1["ment/_common/common_audio/input_card_num_var_nonghyup<br/>━━━━━━━━━━━━━━━━━<br/>카드번호를 입력해 주세요<br/>입력이 끝나면 우물정자(#)를 눌러주세요<br/>단, NH농협카드는 결제가 불가합니다"]
        C2["[TTS] 고객님께서 누르신 카드번호는<br/>1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6 번 입니다"]
        C3["ment/_common/common_audio/input_confirm<br/>━━━━━━━━━━━━━━━━━<br/>맞으면 1번, 틀리면 2번"]
        C1 --> C2 --> C3
    end
    
    C3 --> C_CONFIRM{확인}
    C_CONFIRM -->|1번 맞음| EXPIRE
    C_CONFIRM -->|2번 틀림| C1
    
    subgraph EXPIRE["5. 유효기간 입력"]
        direction TB
        E1["ment/ALLAT_Hangung/input_cardexp_start<br/>━━━━━━━━━━━━━━━━━<br/>신용카드 유효기간 4자리를<br/>카드에 표기된대로 월,연 순서로<br/>입력하여 주십시오"]
        E2["[TTS] 고객님께서 누르신 유효기간은<br/>2025년 12월 입니다"]
        E3["ment/_common/common_audio/input_confirm<br/>━━━━━━━━━━━━━━━━━<br/>맞으면 1번, 틀리면 2번"]
        E1 --> E2 --> E3
    end
    
    E3 --> E_CONFIRM{확인}
    E_CONFIRM -->|1번 맞음| AMT_CHECK{금액 체크}
    E_CONFIRM -->|2번 틀림| E1
    
    AMT_CHECK -->|5만원 미만| PAYMENT[결제 처리]
    AMT_CHECK -->|5만원 이상| INSTALL
    
    subgraph INSTALL["6. 할부개월 선택"]
        direction TB
        I1["ment/_common/common_audio/input_halbu_start<br/>━━━━━━━━━━━━━━━━━<br/>요청하실 할부 개월수를 입력해 주세요<br/>일시불은 0번을 눌러주세요"]
        I2["ment/ALLAT_Hangung/input_halbu_start_ilsibul<br/>━━━━━━━━━━━━━━━━━<br/>신한,삼성,국민,현대카드는 최대 3개월까지<br/>하나카드는 최대 4개월까지<br/>무이자할부가 가능합니다<br/>단, NH농협카드는 결제가 불가하며<br/>우리카드는 BC카드와 별도정책으로 운영됩니다"]
        I3["[TTS] 고객님께서 요청하신<br/>할부 개월수는 3개월 입니다"]
        I4["ment/_common/common_audio/input_confirm<br/>━━━━━━━━━━━━━━━━━<br/>맞으면 1번, 틀리면 2번"]
        I1 --> I2 --> I3 --> I4
    end
    
    I4 --> I_CONFIRM{확인}
    I_CONFIRM -->|1번 맞음| PAYMENT
    I_CONFIRM -->|2번 틀림| I1
```

### 2-5. 결제처리 및 종료

```mermaid
flowchart TD
    subgraph PAYMENT["7. 결제 처리"]
        direction TB
        PAY1[ALLAT PG 결제 연동]
        PAY1 --> PAY_RESULT{결제 결과}
    end
    
    PAY_RESULT -->|성공| SUCCESS
    PAY_RESULT -->|실패| FAIL
    
    subgraph SUCCESS["결제 성공"]
        S1["ment/_common/common_audio/pay_success_msg<br/>━━━━━━━━━━━━━━━━━<br/>결제가 완료 되었습니다"]
    end
    
    subgraph FAIL["결제 실패"]
        F1["[TTS] 고객님, {결과메시지} 이유로 인해"]
        F2["ment/_common/common_audio/pay_fail_msg<br/>━━━━━━━━━━━━━━━━━<br/>결제가 실패되었습니다"]
        F1 --> F2
    end
    
    SUCCESS --> EXIT
    FAIL --> EXIT
    
    subgraph EXIT["8. 종료"]
        direction TB
        EXIT_CHECK{종료 유형}
        EXIT_CHECK -->|정상| EXIT1["ment/_common/common_audio/service_end<br/>━━━━━━━━━━━━━━━━━<br/>이용해 주셔서 감사합니다"]
        EXIT_CHECK -->|에러| EXIT2["ment/_common/common_audio/Error_end<br/>━━━━━━━━━━━━━━━━━<br/>이용방법을 확인해 주시기 바랍니다"]
    end
    
    EXIT1 --> END_CALL([통화 종료])
    EXIT2 --> END_CALL
```

---

## 3. 시스템 에러 처리 Flow

```mermaid
flowchart TD
    subgraph ERRORS["시스템 에러 처리"]
        direction TB
        
        ERR1["ment/TTS_TimeOut<br/>━━━━━━━━━━━━━━━━━<br/>현재 통화량이 많아<br/>지연상황이 발생하고 있습니다"]
        
        ERR2["set_guide 399<br/>━━━━━━━━━━━━━━━━━<br/>시스템 장애 기본 안내"]
        
        ERR3["ment/ALLAT_WOWTV/Tcp_Error<br/>━━━━━━━━━━━━━━━━━<br/>TCP 통신 에러 안내"]
    end
    
    ERR1 --> EXIT_SVC[종료 서비스]
    ERR2 --> EXIT_SVC
    ERR3 --> EXIT_SVC
    
    EXIT_SVC --> END([통화 종료])
```

---

## 4. 입력 오류 처리 Flow

```mermaid
flowchart TD
    subgraph INPUT_ERROR["입력 오류 처리"]
        direction TB
        
        IE1[사용자 입력]
        IE2{유효성 검사}
        
        IE2 -->|유효| NEXT[다음 단계]
        IE2 -->|무효| ERROR["send_error 호출<br/>━━━━━━━━━━━━━━━━━<br/>잘못 누르셨습니다"]
        
        ERROR --> RETRY_CHECK{재시도 횟수}
        RETRY_CHECK -->|초과| EXIT[종료]
        RETRY_CHECK -->|미초과| IE1
    end
```

---

## 5. 전체 멘트 파일 목록

| 순서 | 파일 경로 | 용도 |
|------|-----------|------|
| 1 | `audio/shop_intro/wownet_intro` | 인사말 |
| 2 | `ment/_common/common_audio/input_telnum_start` | 전화번호 입력 안내 |
| 3 | `audio/input_sms_start` | SMS 주문번호 입력 안내 |
| 4 | `audio/input_sms_msg` | 입력하신 주문번호는 |
| 5 | `ment/_common/common_audio/input_confirm` | 확인 (맞으면 1번, 틀리면 2번) |
| 6 | `ment/_common/common_audio/no_order_msg` | 주문 없음 안내 |
| 7 | `ment/ALLAT_WOWTV/Tcp_Error` | TCP 통신 에러 |
| 8 | `ment/_common/common_audio/input_card_num_var_nonghyup` | 카드번호 입력 (농협 불가 안내) |
| 9 | `ment/ALLAT_Hangung/input_cardexp_start` | 유효기간 입력 안내 |
| 10 | `ment/_common/common_audio/input_halbu_start` | 할부개월 입력 안내 |
| 11 | `ment/ALLAT_Hangung/input_halbu_start_ilsibul` | 무이자 할부 카드사 안내 |
| 12 | `ment/_common/common_audio/input_nohalbu_msg` | 일시불 선택 안내 |
| 13 | `ment/_common/common_audio/input_halbu_err` | 할부개월 오류 안내 |
| 14 | `ment/_common/common_audio/pay_success_msg` | 결제 완료 |
| 15 | `ment/_common/common_audio/pay_fail_msg` | 결제 실패 |
| 16 | `ment/_common/common_audio/service_end` | 정상 종료 |
| 17 | `ment/_common/common_audio/Error_end` | 에러 종료 |
| 18 | `ment/TTS_TimeOut` | 통화량 과다 |

---

*문서 작성일: 2026-01-29*
*소스 파일: ALLAT_Stockwin_Quick_New_Scenario.cpp*
