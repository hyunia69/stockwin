<!--METADATA TYPE="typelib" FILE="C:\program Files\Common Files\System\ado\msado15.dll"-->
<%
' ============================================
' StockWin 결제 시나리오 등록 API
' ============================================

Dim strConnect
strConnect = "Provider=SQLOLEDB.1;Password=medi@ford;Persist Security Info=True;User ID=sa;Initial Catalog=arspg_web;Data Source=211.196.157.121"

Dim MODE_VALUE, PG_CODE_VALUE, DLL_NAME_VALUE
MODE_VALUE = "hangung2^alphago_hankyung"
PG_CODE_VALUE = "allat"
DLL_NAME_VALUE = "ALLAT_StockWin_Billkey_Easy_New_Scenario.dll"

' 요청 파라미터 수신
Dim strMode, shop_id, ars_tel_no, scenario_type, arrribute_type
Dim amount, cc_pord_desc, startdtm

strMode = Trim(Request.Form("mode"))
shop_id = Trim(Request.Form("shop_id"))
ars_tel_no = Trim(Request.Form("ars_tel_no"))
scenario_type = Trim(Request.Form("scenario_type"))
arrribute_type = Trim(Request.Form("arrribute_type"))
amount = Trim(Request.Form("amount"))
cc_pord_desc = Trim(Request.Form("cc_pord_desc"))
startdtm = Trim(Request.Form("startdtm"))

' mode 검증
If strMode <> MODE_VALUE Then
    Response.Write ars_tel_no & "||0001|mode 값이 올바르지 않습니다."
    Response.End
End If

' shop_id 검증
If shop_id = "" Then
    Response.Write ars_tel_no & "||0002|shop_id가 누락되었습니다."
    Response.End
End If

If shop_id <> "arsstockwin" And shop_id <> "arsstockwin1" And shop_id <> "arsstockwin2" Then
    Response.Write ars_tel_no & "||0002|유효하지 않은 shop_id입니다."
    Response.End
End If

' ars_tel_no 검증
If ars_tel_no = "" Then
    Response.Write ars_tel_no & "||0003|ars_tel_no가 누락되었습니다."
    Response.End
End If

' scenario_type 검증
If scenario_type = "" Then
    Response.Write ars_tel_no & "||0004|scenario_type이 누락되었습니다."
    Response.End
End If

If scenario_type <> "CQS" And scenario_type <> "CIP" Then
    Response.Write ars_tel_no & "||0004|유효하지 않은 scenario_type입니다."
    Response.End
End If

' arrribute_type 검증
If arrribute_type = "" Then
    Response.Write ars_tel_no & "||0005|arrribute_type이 누락되었습니다."
    Response.End
End If

' amount 검증
If amount = "" Then
    Response.Write ars_tel_no & "||0006|amount가 누락되었습니다."
    Response.End
End If

' cc_pord_desc 검증
If cc_pord_desc = "" Then
    Response.Write ars_tel_no & "||0007|cc_pord_desc가 누락되었습니다."
    Response.End
End If

' startdtm 검증 (ADVRESERVD일 때만 필수)
If UCase(arrribute_type) = "ADVRESERVD" Then
    If startdtm = "" Then
        Response.Write ars_tel_no & "||0008|ADVRESERVD 속성에는 startdtm이 필수입니다."
        Response.End
    End If
End If

' 비즈니스 로직 처리
Dim service_name, product_code, dnis_descript, pos

pos = InStr(cc_pord_desc, "^")
If pos > 0 Then
    service_name = Left(cc_pord_desc, pos - 1)
    product_code = Mid(cc_pord_desc, pos + 1)
Else
    service_name = cc_pord_desc
    product_code = ""
End If

' JSON 생성
dnis_descript = "{""amount"":""" & amount & """,""attr"":""" & arrribute_type & """"
If startdtm <> "" Then
    dnis_descript = dnis_descript & ",""startdtm"":""" & startdtm & """"
End If
If product_code <> "" Then
    dnis_descript = dnis_descript & ",""product_code"":""" & product_code & """"
End If
dnis_descript = dnis_descript & "}"

' DB 처리
Dim dbCon, rs, qry, dnisCount

On Error Resume Next

Set dbCon = Server.CreateObject("ADODB.Connection")
dbCon.Open strConnect

If Err.Number <> 0 Then
    Response.Write ars_tel_no & "||9001|데이터베이스 연결 실패: " & Err.Description
    Response.End
End If

' 중복 체크
qry = "SELECT COUNT(*) AS cnt FROM COMMON_DNIS_INFO WHERE ARS_DNIS = '" & Replace(ars_tel_no, "'", "''") & "'"
Set rs = dbCon.Execute(qry)

If Err.Number <> 0 Then
    dbCon.Close
    Set dbCon = Nothing
    Response.Write ars_tel_no & "||9001|데이터베이스 조회 실패: " & Err.Description
    Response.End
End If

dnisCount = rs("cnt")
rs.Close
Set rs = Nothing

If dnisCount = 0 Then
    ' INSERT
    qry = "INSERT INTO COMMON_DNIS_INFO (" & _
          "ADMIN_ID, ARS_DNIS, ARS_TYPE, DLL_NAME, DNIS_DESCRIPT, " & _
          "PG_CODE, SERVOCE_NAME, USE_YN, INVALID_NO_YN, " & _
          "WRITE_DATE, WRITE_DT, WRITE_ID" & _
          ") VALUES (" & _
          "'" & Replace(shop_id, "'", "''") & "', " & _
          "'" & Replace(ars_tel_no, "'", "''") & "', " & _
          "'" & Replace(scenario_type, "'", "''") & "', " & _
          "'" & Replace(DLL_NAME_VALUE, "'", "''") & "', " & _
          "'" & Replace(dnis_descript, "'", "''") & "', " & _
          "'" & PG_CODE_VALUE & "', " & _
          "'" & Replace(service_name, "'", "''") & "', " & _
          "'Y', 'N', " & _
          "GETDATE(), GETDATE(), 'SYSTEM'" & _
          ")"

    dbCon.Execute qry

    If Err.Number <> 0 Then
        dbCon.Close
        Set dbCon = Nothing
        Response.Write ars_tel_no & "||9001|데이터 등록 실패: " & Err.Description
        Response.End
    End If
Else
    ' UPDATE
    qry = "UPDATE COMMON_DNIS_INFO SET " & _
          "ADMIN_ID = '" & Replace(shop_id, "'", "''") & "', " & _
          "ARS_TYPE = '" & Replace(scenario_type, "'", "''") & "', " & _
          "DLL_NAME = '" & Replace(DLL_NAME_VALUE, "'", "''") & "', " & _
          "DNIS_DESCRIPT = '" & Replace(dnis_descript, "'", "''") & "', " & _
          "SERVOCE_NAME = '" & Replace(service_name, "'", "''") & "', " & _
          "WRITE_DT = GETDATE() " & _
          "WHERE ARS_DNIS = '" & Replace(ars_tel_no, "'", "''") & "'"

    dbCon.Execute qry

    If Err.Number <> 0 Then
        dbCon.Close
        Set dbCon = Nothing
        Response.Write ars_tel_no & "||9001|데이터 수정 실패: " & Err.Description
        Response.End
    End If
End If

dbCon.Close
Set dbCon = Nothing

On Error GoTo 0

' 성공 응답
Dim item_cd
item_cd = product_code
If item_cd = "" Then
    item_cd = service_name
End If

Response.Write ars_tel_no & "|" & item_cd & "|0000|등록이 완료되었습니다."
%>
