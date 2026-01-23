# 일반결제 시나리오 구현 설계서

**버전**: 1.0
**작성일**: 2026-01-24
**목적**: NEW_SPEC_SCENARIO.md 일반결제 시나리오 Gap 구현

---

## 1. 구현 범위

### 1.1 구현 대상

| 항목 | 설명 | 상태 |
|------|------|------|
| 결제금액 안내 멘트 4분기 | 쿠폰/보너스캐시 적용 여부별 TTS 분기 | 🔨 구현 예정 |
| 상품유형별 해지조건 안내 | 일반/태블릿/교육 상품별 해지조건 | 🔨 구현 예정 |
| 쿠폰/보너스캐시 소멸 안내 | 해지 시 쿠폰/보너스캐시 소멸 경고 | 🔨 구현 예정 |

### 1.2 구현 제외

| 항목 | 사유 |
|------|------|
| NH농협카드 안내 | 요청에 따라 제외 |
| 암호화 동의 안내 | 요청에 따라 제외 |

---

## 2. 상품유형 분류 (categoryId_2nd)

### 2.1 상품유형 정의

`categoryId_2nd` 필드를 상품유형으로 사용합니다.

| categoryId_2nd 값 | 상품유형 | 설명 |
|-------------------|----------|------|
| `SERVICE` | 일반상품 | 기본값 (값이 없거나 빈 문자열일 때) |
| `TABLET` | 태블릿 제공 상품 | 태블릿 포함 상품 |
| `EDUCATION` | 교육상품 | 교육/강의 상품 |
| 기타 | 일반상품 | 알 수 없는 값은 일반상품으로 처리 |

### 2.2 기본값 처리

```
if (categoryId_2nd가 비어있음 또는 NULL) {
    categoryId_2nd = "SERVICE"
}
```

---

## 3. 파일별 수정 사항

### 3.1 ALLAT_Stockwin_Quick_New_Scenario.h

**위치**: 멤버 변수 섹션 (라인 111 근처)

**추가할 멤버 변수**:
```cpp
// 상품유형 (categoryId_2nd 기반)
char  m_szCategoryId[17];        // 상품유형 (SERVICE/TABLET/EDUCATION)
```

---

### 3.2 WowTvSocket.cpp

**위치**: `PL_InfoOrderReq_Process()` 함수, 데이터 저장 섹션 (라인 1104 근처)

**추가할 코드** (보너스캐시 저장 후):
```cpp
// 상품유형 (categoryId_2nd)
// 값이 비어있으면 기본값 "SERVICE" 사용
if (strlen(plInfo.categoryId_2nd) > 0) {
    strncpy_s(pScenario->m_szCategoryId, sizeof(pScenario->m_szCategoryId),
              plInfo.categoryId_2nd, sizeof(pScenario->m_szCategoryId) - 1);
} else {
    strncpy_s(pScenario->m_szCategoryId, sizeof(pScenario->m_szCategoryId),
              "SERVICE", sizeof(pScenario->m_szCategoryId) - 1);
}
xprintf("[CH:%03d] PL_InfoOrderReq: 상품유형(categoryId_2nd)=%s", ch, pScenario->m_szCategoryId);
```

---

### 3.3 ALLAT_Stockwin_Quick_New_Scenario.cpp

**위치**: `ALLAT_getOrderInfo()` 함수, state=9 (라인 1492~1511)

#### 3.3.1 현재 코드 (단일 멘트)

```cpp
case 9:
    if (TTS_Play)
    {
        setPostfunc(POST_NET, ALLAT_getOrderInfo, 10, 0);
        return TTS_Play((*lpmt)->chanID, 92,
            "%s 고객님, %s에서 , 주문하신 %s의 결제하실 금액은 %d원 입니다. "
            "서비스 중도해지시 해지일까지 일수 만큼의 이용요금과 ..."
            "동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
            pScenario->m_szCC_name,
            pScenario->m_szMx_name,
            pScenario->m_szCC_Prod_Desc,
            pScenario->m_nAmount);
    }
```

#### 3.3.2 변경 후 코드 (4분기 + 상품유형별)

```cpp
case 9:
    if (TTS_Play)
    {
        setPostfunc(POST_NET, ALLAT_getOrderInfo, 10, 0);

        // ================================================================
        // 결제금액 안내 멘트 생성
        // ================================================================
        char szPaymentMent[1024] = {0};

        // 할인 적용 여부에 따른 결제금액 안내
        bool bCoupon = (strcmp(pScenario->m_szCouponUseFlag, "Y") == 0 &&
                        strlen(pScenario->m_szCouponName) > 0);
        bool bBonusCash = (pScenario->m_nBonusCashUseAmt > 0);

        if (bCoupon && bBonusCash) {
            // 쿠폰 + 보너스캐시 둘 다 적용
            sprintf_s(szPaymentMent, sizeof(szPaymentMent),
                "%s 고객님, %s에서, 주문하신 %s의 결제 금액은 "
                "%s님께서 보유하신 %s 쿠폰과 보너스 캐시 %d원이 적용되어 "
                "최종 결제 금액은 %d원입니다.",
                pScenario->m_szCC_name,
                pScenario->m_szMx_name,
                pScenario->m_szCC_Prod_Desc,
                pScenario->m_szCC_name,
                pScenario->m_szCouponName,
                pScenario->m_nBonusCashUseAmt,
                pScenario->m_nAmount);
        }
        else if (bCoupon) {
            // 쿠폰만 적용
            sprintf_s(szPaymentMent, sizeof(szPaymentMent),
                "%s 고객님, %s에서, 주문하신 %s의 결제 금액은 "
                "%s님께서 보유하신 %s 쿠폰이 적용되어 "
                "최종 결제 금액은 %d원입니다.",
                pScenario->m_szCC_name,
                pScenario->m_szMx_name,
                pScenario->m_szCC_Prod_Desc,
                pScenario->m_szCC_name,
                pScenario->m_szCouponName,
                pScenario->m_nAmount);
        }
        else if (bBonusCash) {
            // 보너스캐시만 적용
            sprintf_s(szPaymentMent, sizeof(szPaymentMent),
                "%s 고객님, %s에서, 주문하신 %s의 결제 금액은 "
                "%s님께서 보유하신 보너스 캐시 %d원이 적용되어 "
                "최종 결제 금액은 %d원입니다.",
                pScenario->m_szCC_name,
                pScenario->m_szMx_name,
                pScenario->m_szCC_Prod_Desc,
                pScenario->m_szCC_name,
                pScenario->m_nBonusCashUseAmt,
                pScenario->m_nAmount);
        }
        else {
            // 할인 없음 (기본)
            sprintf_s(szPaymentMent, sizeof(szPaymentMent),
                "%s 고객님, %s에서, 주문하신 %s의 "
                "결제하실 금액은 %d원입니다.",
                pScenario->m_szCC_name,
                pScenario->m_szMx_name,
                pScenario->m_szCC_Prod_Desc,
                pScenario->m_nAmount);
        }

        // ================================================================
        // 상품유형별 해지조건 안내 멘트 생성
        // ================================================================
        char szTermsMent[1024] = {0};

        if (strcmp(pScenario->m_szCategoryId, "TABLET") == 0) {
            // 태블릿 제공 상품
            strcpy_s(szTermsMent, sizeof(szTermsMent),
                "박스 개봉 후 제품 불량을 제외하고는 교환 및 반품이 불가합니다.");
        }
        else if (strcmp(pScenario->m_szCategoryId, "EDUCATION") == 0) {
            // 교육상품
            strcpy_s(szTermsMent, sizeof(szTermsMent),
                "본 상품은 교육 상품으로 결제 후 해지가 불가능합니다. "
                "또한, 무료로 제공되는 서비스는 중도 해지 및 일시 정지 파일 양도가 불가합니다.");
        }
        else {
            // 일반상품 (SERVICE 또는 기타)
            strcpy_s(szTermsMent, sizeof(szTermsMent),
                "서비스 중도해지 시 해지일까지 이용요금과 해지수수료 10퍼센트와 "
                "제공받으신 사은품 정가가 함께 차감됩니다.");

            // 쿠폰/보너스캐시 사용 시 소멸 안내 추가
            if (bCoupon || bBonusCash) {
                strcat_s(szTermsMent, sizeof(szTermsMent),
                    " 또한 적용된 쿠폰 및 보너스 캐시는 해지 시 소멸됩니다.");
            }
        }

        // ================================================================
        // 투자 유의사항 안내
        // ================================================================
        const char* szInvestMent =
            "또한, 한국경제TV와 파트너는 금융투자업자가 아닌 유사투자자문업자로 "
            "개별 투자 상담과 자금 운용이 불가하며, "
            "원금 손실이 발생할 수 있고 그 손실은 투자자에게 귀속됩니다.";

        // ================================================================
        // 동의 확인 멘트
        // ================================================================
        const char* szConfirmMent =
            "동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.";

        // ================================================================
        // 전체 TTS 멘트 조합
        // ================================================================
        char szFullMent[4096] = {0};
        sprintf_s(szFullMent, sizeof(szFullMent),
            "%s %s %s %s",
            szPaymentMent,
            szTermsMent,
            szInvestMent,
            szConfirmMent);

        return TTS_Play((*lpmt)->chanID, 92, "%s", szFullMent);
    }
    else
    {
        set_guide(399);
        setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
        return send_guide(NODTMF);
    }
```

---

## 4. TTS 멘트 상세

### 4.1 결제금액 안내 (4가지 분기)

| 조건 | 멘트 |
|------|------|
| **할인 없음** | `[고객명] 고객님, [가맹점명]에서, 주문하신 [상품명]의 결제하실 금액은 [금액]원입니다.` |
| **쿠폰만** | `... [고객명]님께서 보유하신 [쿠폰명] 쿠폰이 적용되어 최종 결제 금액은 [금액]원입니다.` |
| **보너스캐시만** | `... [고객명]님께서 보유하신 보너스 캐시 [금액]원이 적용되어 최종 결제 금액은 [금액]원입니다.` |
| **둘 다** | `... [고객명]님께서 보유하신 [쿠폰명] 쿠폰과 보너스 캐시 [금액]원이 적용되어 최종 결제 금액은 [금액]원입니다.` |

### 4.2 상품유형별 해지조건 안내

| categoryId_2nd | 멘트 |
|----------------|------|
| **SERVICE** (기본) | `서비스 중도해지 시 해지일까지 이용요금과 해지수수료 10퍼센트와 제공받으신 사은품 정가가 함께 차감됩니다.` |
| **TABLET** | `박스 개봉 후 제품 불량을 제외하고는 교환 및 반품이 불가합니다.` |
| **EDUCATION** | `본 상품은 교육 상품으로 결제 후 해지가 불가능합니다. 또한, 무료로 제공되는 서비스는 중도 해지 및 일시 정지 파일 양도가 불가합니다.` |

### 4.3 쿠폰/보너스캐시 소멸 안내

**조건**: 일반상품(SERVICE)이면서 쿠폰 또는 보너스캐시 사용 시

**멘트**: `또한 적용된 쿠폰 및 보너스 캐시는 해지 시 소멸됩니다.`

### 4.4 투자 유의사항 안내 (공통)

> 또한, 한국경제TV와 파트너는 금융투자업자가 아닌 유사투자자문업자로 개별 투자 상담과 자금 운용이 불가하며, 원금 손실이 발생할 수 있고 그 손실은 투자자에게 귀속됩니다.

---

## 5. 데이터 흐름

```
PayLetter API 응답
    │
    ├─ couponUseFlag (Y/N)
    ├─ couponName (쿠폰명)
    ├─ bonusCashUseAmt (보너스캐시 금액)
    ├─ categoryId_2nd (상품유형) ← 비어있으면 "SERVICE"
    │
    ▼
WowTvSocket.cpp::PL_InfoOrderReq_Process()
    │
    ├─ m_szCouponUseFlag
    ├─ m_szCouponName
    ├─ m_nBonusCashUseAmt
    ├─ m_szCategoryId ← 신규 추가
    │
    ▼
ALLAT_getOrderInfo() state=9
    │
    ├─ 결제금액 안내 (4분기)
    ├─ 상품유형별 해지조건
    ├─ 쿠폰/보너스캐시 소멸 안내
    ├─ 투자 유의사항
    └─ 동의 확인
```

---

## 6. 테스트 시나리오

### 6.1 결제금액 안내 테스트

| 테스트 케이스 | couponUseFlag | couponName | bonusCashUseAmt | 예상 멘트 |
|--------------|---------------|------------|-----------------|-----------|
| TC-01 | N | - | 0 | 기본 멘트 |
| TC-02 | Y | 10%할인쿠폰 | 0 | 쿠폰 적용 멘트 |
| TC-03 | N | - | 1000 | 보너스캐시 적용 멘트 |
| TC-04 | Y | 10%할인쿠폰 | 1000 | 쿠폰+보너스캐시 멘트 |

### 6.2 상품유형별 테스트

| 테스트 케이스 | categoryId_2nd | 예상 해지조건 멘트 |
|--------------|----------------|-------------------|
| TC-05 | (빈값) | 일반상품 해지조건 + "SERVICE" 설정 |
| TC-06 | SERVICE | 일반상품 해지조건 |
| TC-07 | TABLET | 태블릿 반품불가 안내 |
| TC-08 | EDUCATION | 교육상품 해지불가 안내 |
| TC-09 | UNKNOWN | 일반상품 해지조건 (기본 처리) |

### 6.3 쿠폰/보너스캐시 소멸 안내 테스트

| 테스트 케이스 | categoryId_2nd | 할인 적용 | 소멸 안내 포함 |
|--------------|----------------|-----------|----------------|
| TC-10 | SERVICE | 쿠폰 사용 | ✅ 포함 |
| TC-11 | SERVICE | 보너스캐시 사용 | ✅ 포함 |
| TC-12 | SERVICE | 할인 없음 | ❌ 미포함 |
| TC-13 | TABLET | 쿠폰 사용 | ❌ 미포함 |
| TC-14 | EDUCATION | 쿠폰 사용 | ❌ 미포함 |

---

## 7. 수정 파일 목록

| 파일 | 수정 내용 | 영향도 |
|------|-----------|--------|
| `ALLAT_Stockwin_Quick_New_Scenario.h` | `m_szCategoryId` 멤버 변수 추가 | 낮음 |
| `WowTvSocket.cpp` | `categoryId_2nd` 매핑 로직 추가 | 낮음 |
| `ALLAT_Stockwin_Quick_New_Scenario.cpp` | `ALLAT_getOrderInfo` state=9 TTS 분기 로직 | 중간 |

---

## 8. 검증 체크리스트

- [ ] `categoryId_2nd` 빈 값일 때 "SERVICE" 기본값 설정 확인
- [ ] 쿠폰 사용 시 쿠폰명 TTS 출력 확인
- [ ] 보너스캐시 사용 시 금액 TTS 출력 확인
- [ ] 쿠폰+보너스캐시 동시 사용 시 TTS 출력 확인
- [ ] TABLET 상품 해지조건 TTS 확인
- [ ] EDUCATION 상품 해지조건 TTS 확인
- [ ] 일반상품 쿠폰 사용 시 소멸 안내 TTS 확인
- [ ] 투자 유의사항 TTS 출력 확인
- [ ] DTMF 1/2 입력 처리 정상 동작 확인
