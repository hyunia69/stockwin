<%@ CodePage=65001 Language="VBScript" %>
<!--METADATA TYPE="typelib" FILE="C:\program Files\Common Files\System\ado\msado15.dll"-->
<%
' ============================================
' StockWin 주문 결제정보 전달 API
' ============================================
' 인코딩 처리:
'   - Client 요청: UTF-8 JSON
'   - Response 응답: UTF-8 JSON
'   - DB 저장: EUC-KR (varchar 컬럼, SQL Server 자동 변환)
' ============================================

Response.CharSet = "UTF-8"
Response.ContentType = "application/json; charset=UTF-8"

' ============================================
' UTF-8 처리 함수 (ADODB.Stream 사용)
' ============================================

' 바이너리 데이터를 UTF-8로 디코딩하여 유니코드 문자열 반환
Function BinaryToUTF8String(binaryData)
    Dim objStream
    Set objStream = Server.CreateObject("ADODB.Stream")

    ' 바이너리 데이터를 스트림에 쓰기
    objStream.Type = 1  ' adTypeBinary
    objStream.Open
    objStream.Write binaryData

    ' 스트림 위치를 처음으로 이동
    objStream.Position = 0

    ' UTF-8 텍스트로 읽기
    objStream.Type = 2  ' adTypeText
    objStream.Charset = "UTF-8"

    BinaryToUTF8String = objStream.ReadText

    objStream.Close
    Set objStream = Nothing
End Function

' ============================================
' JSON 파싱 함수들 (간단한 구현)
' ============================================

' JSON 문자열에서 특정 키의 값을 추출
Function GetJsonValue(jsonStr, key)
    Dim pattern, startPos, endPos, value
    GetJsonValue = ""

    ' "key" : "value" 또는 "key": "value" 패턴 찾기
    pattern = """" & key & """"
    startPos = InStr(1, jsonStr, pattern, vbTextCompare)

    If startPos > 0 Then
        ' : 다음의 값 찾기
        startPos = InStr(startPos, jsonStr, ":")
        If startPos > 0 Then
            startPos = startPos + 1

            ' 공백 스킵
            Do While Mid(jsonStr, startPos, 1) = " " Or Mid(jsonStr, startPos, 1) = vbTab
                startPos = startPos + 1
            Loop

            ' 값이 따옴표로 시작하는 경우 (문자열)
            If Mid(jsonStr, startPos, 1) = """" Then
                startPos = startPos + 1
                endPos = startPos

                ' 닫는 따옴표 찾기 (이스케이프된 따옴표 처리)
                Do While endPos <= Len(jsonStr)
                    If Mid(jsonStr, endPos, 1) = """" Then
                        If Mid(jsonStr, endPos - 1, 1) <> "\" Then
                            Exit Do
                        End If
                    End If
                    endPos = endPos + 1
                Loop

                value = Mid(jsonStr, startPos, endPos - startPos)
                ' 이스케이프된 문자 처리
                value = Replace(value, "\""", """")
                value = Replace(value, "\\", "\")
                GetJsonValue = value
            Else
                ' 숫자나 다른 값
                endPos = startPos
                Do While endPos <= Len(jsonStr)
                    Dim ch
                    ch = Mid(jsonStr, endPos, 1)
                    If ch = "," Or ch = "}" Or ch = vbCr Or ch = vbLf Then
                        Exit Do
                    End If
                    endPos = endPos + 1
                Loop
                GetJsonValue = Trim(Mid(jsonStr, startPos, endPos - startPos))
            End If
        End If
    End If
End Function

' JSON 응답 생성
Function MakeJsonResponse(errorCode, message, orderNo)
    Dim json
    json = "{" & vbCrLf
    json = json & "  ""error_cd"": """ & errorCode & """," & vbCrLf
    json = json & "  ""message"": """ & Replace(message, """", "\""") & """," & vbCrLf
    json = json & "  ""order_no"": """ & orderNo & """" & vbCrLf
    json = json & "}"
    MakeJsonResponse = json
End Function

' UTF-8로 응답 출력 (ADODB.Stream 사용)
Sub WriteUTF8Response(text)
    Dim objStream
    Set objStream = Server.CreateObject("ADODB.Stream")

    ' 유니코드 문자열을 UTF-8 바이너리로 변환
    objStream.Type = 2  ' adTypeText
    objStream.Charset = "UTF-8"
    objStream.Open
    objStream.WriteText text

    ' 바이너리로 읽어서 Response에 출력
    objStream.Position = 0
    objStream.Type = 1  ' adTypeBinary
    objStream.Position = 3  ' UTF-8 BOM(3바이트) 건너뛰기

    Response.BinaryWrite objStream.Read

    objStream.Close
    Set objStream = Nothing
End Sub

' ============================================
' Raw POST 데이터 읽기 및 JSON 파싱
' ============================================

Dim postBytes, jsonData
If Request.TotalBytes > 0 Then
    postBytes = Request.BinaryRead(Request.TotalBytes)
    jsonData = BinaryToUTF8String(postBytes)
Else
    jsonData = ""
End If

' ============================================
' 설정 및 파라미터 처리
' ============================================

Dim strConnect
strConnect = "Provider=SQLOLEDB.1;Password=medi@ford;Persist Security Info=True;User ID=sa;Initial Catalog=arspg_web;Data Source=211.196.157.121"

Dim MODE_VALUE
MODE_VALUE = "hangung^alphago_hankyung"

' JSON에서 파라미터 추출
Dim strMode, shop_id, amount, phone_no, verify_num
Dim request_type, alert_show, order_no, cc_name, cc_pord_desc

strMode = Trim(GetJsonValue(jsonData, "mode"))
shop_id = Trim(GetJsonValue(jsonData, "shop_id"))
amount = Trim(GetJsonValue(jsonData, "amount"))
phone_no = Trim(GetJsonValue(jsonData, "phone_no"))
verify_num = Trim(GetJsonValue(jsonData, "verify_num"))
request_type = Trim(GetJsonValue(jsonData, "request_type"))
alert_show = Trim(GetJsonValue(jsonData, "alert_show"))
order_no = Trim(GetJsonValue(jsonData, "order_no"))
cc_name = Trim(GetJsonValue(jsonData, "cc_name"))
cc_pord_desc = Trim(GetJsonValue(jsonData, "cc_pord_desc"))

' ============================================
' 파라미터 검증
' ============================================

' mode 검증
If strMode <> MODE_VALUE Then
    WriteUTF8Response MakeJsonResponse("0001", "mode 값이 올바르지 않습니다.", order_no)
    Response.End
End If

' shop_id 검증
If shop_id = "" Then
    WriteUTF8Response MakeJsonResponse("0002", "shop_id가 누락되었습니다.", order_no)
    Response.End
End If

If shop_id <> "arsstockwin" And shop_id <> "arsstockwin1" And shop_id <> "arsstockwin2" Then
    WriteUTF8Response MakeJsonResponse("0002", "유효하지 않은 shop_id입니다.", order_no)
    Response.End
End If

' amount 검증
If amount = "" Then
    WriteUTF8Response MakeJsonResponse("0003", "amount가 누락되었습니다.", order_no)
    Response.End
End If

' phone_no 검증
If phone_no = "" Then
    WriteUTF8Response MakeJsonResponse("0004", "phone_no가 누락되었습니다.", order_no)
    Response.End
End If

' verify_num 검증
If verify_num = "" Then
    WriteUTF8Response MakeJsonResponse("0005", "verify_num이 누락되었습니다.", order_no)
    Response.End
End If

' request_type 검증
If request_type = "" Then
    WriteUTF8Response MakeJsonResponse("0006", "request_type이 누락되었습니다.", order_no)
    Response.End
End If

' order_no 검증
If order_no = "" Then
    WriteUTF8Response MakeJsonResponse("0007", "order_no가 누락되었습니다.", order_no)
    Response.End
End If

' cc_name 검증
If cc_name = "" Then
    WriteUTF8Response MakeJsonResponse("0008", "cc_name이 누락되었습니다.", order_no)
    Response.End
End If

' cc_pord_desc 검증
If cc_pord_desc = "" Then
    WriteUTF8Response MakeJsonResponse("0009", "cc_pord_desc가 누락되었습니다.", order_no)
    Response.End
End If

' ============================================
' 비즈니스 로직 처리
' ============================================

Dim service_name, item_code, pos

' cc_pord_desc에서 상품명과 아이템코드 분리 (형식: 상품명^패키지번호)
pos = InStr(cc_pord_desc, "^")
If pos > 0 Then
    service_name = Left(cc_pord_desc, pos - 1)
    item_code = Mid(cc_pord_desc, pos + 1)
Else
    service_name = cc_pord_desc
    item_code = ""
End If

' MX_ISSUE_NO 생성 (order_no 기반 고유 ID)
Dim mx_issue_no
mx_issue_no = order_no & "_" & Year(Now) & Right("0" & Month(Now), 2) & Right("0" & Day(Now), 2) & Right("0" & Hour(Now), 2) & Right("0" & Minute(Now), 2) & Right("0" & Second(Now), 2)

' ============================================
' DB 처리
' ============================================

Dim dbCon, rs, qry, orderCount

On Error Resume Next

Set dbCon = Server.CreateObject("ADODB.Connection")
dbCon.Open strConnect

If Err.Number <> 0 Then
    WriteUTF8Response MakeJsonResponse("9001", "데이터베이스 연결 실패: " & Err.Description, order_no)
    Response.End
End If

' 중복 주문번호 체크
qry = "SELECT COUNT(*) AS cnt FROM ALLAT_SHOP_ORDER WHERE MX_ISSUE_NO LIKE '" & Replace(order_no, "'", "''") & "%'"
Set rs = dbCon.Execute(qry)

If Err.Number <> 0 Then
    dbCon.Close
    Set dbCon = Nothing
    WriteUTF8Response MakeJsonResponse("9001", "데이터베이스 조회 실패: " & Err.Description, order_no)
    Response.End
End If

orderCount = rs("cnt")
rs.Close
Set rs = Nothing

If orderCount > 0 Then
    ' 이미 존재하는 주문번호 - UPDATE
    qry = "UPDATE ALLAT_SHOP_ORDER SET " & _
          "MX_NAME = N'" & Replace(cc_name, "'", "''") & "', " & _
          "MX_ID = '" & Replace(phone_no, "'", "''") & "', " & _
          "MX_OPT = '" & Replace(request_type, "'", "''") & "', " & _
          "ADMIN_ID = '" & Replace(shop_id, "'", "''") & "', " & _
          "CC_NAME = N'" & Replace(cc_name, "'", "''") & "', " & _
          "CC_PORD_DESC = N'" & Replace(cc_pord_desc, "'", "''") & "', " & _
          "AMOUNT = " & CLng(amount) & ", " & _
          "PHONE_NO = '" & Replace(phone_no, "'", "''") & "', " & _
          "AUTH_NO = '" & Replace(verify_num, "'", "''") & "', " & _
          "REQUEST_TYPE = '" & Replace(request_type, "'", "''") & "', " & _
          "ITEM_CODE = N'" & Replace(item_code, "'", "''") & "', " & _
          "REPLY_CODE = '0000', " & _
          "REPLY_MESSAGE = N'주문정보 수정 완료' " & _
          "WHERE MX_ISSUE_NO LIKE '" & Replace(order_no, "'", "''") & "%'"

    dbCon.Execute qry

    If Err.Number <> 0 Then
        dbCon.Close
        Set dbCon = Nothing
        WriteUTF8Response MakeJsonResponse("9001", "데이터 수정 실패: " & Err.Description, order_no)
        Response.End
    End If

    dbCon.Close
    Set dbCon = Nothing

    On Error GoTo 0

    ' 성공 응답 (수정)
    WriteUTF8Response MakeJsonResponse("0000", "주문정보가 수정되었습니다.", order_no)
Else
    ' 신규 등록 - INSERT
    qry = "INSERT INTO ALLAT_SHOP_ORDER (" & _
          "MX_ISSUE_NO, MX_NAME, MX_ID, MX_OPT, ADMIN_ID, " & _
          "CC_NAME, CC_PORD_DESC, AMOUNT, PHONE_NO, AUTH_NO, " & _
          "REQUEST_TYPE, REPLY_CODE, REPLY_MESSAGE, REG_DATE, AUTO_INPUT, ITEM_CODE" & _
          ") VALUES (" & _
          "'" & Replace(mx_issue_no, "'", "''") & "', " & _
          "N'" & Replace(cc_name, "'", "''") & "', " & _
          "'" & Replace(phone_no, "'", "''") & "', " & _
          "'" & Replace(request_type, "'", "''") & "', " & _
          "'" & Replace(shop_id, "'", "''") & "', " & _
          "N'" & Replace(cc_name, "'", "''") & "', " & _
          "N'" & Replace(cc_pord_desc, "'", "''") & "', " & _
          CLng(amount) & ", " & _
          "'" & Replace(phone_no, "'", "''") & "', " & _
          "'" & Replace(verify_num, "'", "''") & "', " & _
          "'" & Replace(request_type, "'", "''") & "', " & _
          "'0000', " & _
          "N'주문정보 등록 완료', " & _
          "GETDATE(), " & _
          "'N', " & _
          "N'" & Replace(item_code, "'", "''") & "'" & _
          ")"

    dbCon.Execute qry

    If Err.Number <> 0 Then
        dbCon.Close
        Set dbCon = Nothing
        WriteUTF8Response MakeJsonResponse("9001", "데이터 등록 실패: " & Err.Description, order_no)
        Response.End
    End If

    dbCon.Close
    Set dbCon = Nothing

    On Error GoTo 0

    ' 성공 응답 (등록)
    WriteUTF8Response MakeJsonResponse("0000", "주문정보가 등록되었습니다.", order_no)
End If
%>
