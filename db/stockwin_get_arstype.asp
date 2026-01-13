<!--METADATA TYPE="typelib" FILE="C:\program Files\Common Files\System\ado\msado15.dll"-->
<%
' ============================================
' StockWin 결제 유형 조회 API
' ============================================
' 인코딩 처리:
'   - Client 요청: UTF-8 (GET 파라미터)
'   - Response 응답: UTF-8
'   - DB 조회: EUC-KR (varchar 컬럼, SQL Server 자동 변환)
' ============================================

Response.CharSet="UTF-8"
Response.ContentType="text/html;charset=UTF-8"

' ============================================
' 설정
' ============================================

Dim strConnect
strConnect = "Provider=SQLOLEDB.1;Password=medi@ford;Persist Security Info=True;User ID=sa;Initial Catalog=arspg_web;Data Source=211.196.157.121"

Dim MODE_VALUE
MODE_VALUE = "hangung2^alphago_hankyung"

' ============================================
' 요청 파라미터 수신 (GET)
' ============================================

Dim strMode, ars_tel_no

strMode = Trim(Request.QueryString("mode"))
ars_tel_no = Trim(Request.QueryString("ars_tel_no"))

' mode 검증
If strMode <> MODE_VALUE Then
    Response.Write ars_tel_no & "||0001|mode 값이 올바르지 않습니다."
    Response.End
End If

' ars_tel_no 검증
If ars_tel_no = "" Then
    Response.Write "||0002|ars_tel_no가 누락되었습니다."
    Response.End
End If

' ============================================
' DB 조회
' ============================================

Dim dbCon, rs, qry, type_cd_list

On Error Resume Next

Set dbCon = Server.CreateObject("ADODB.Connection")
dbCon.Open strConnect

If Err.Number <> 0 Then
    Response.Write ars_tel_no & "||9001|데이터베이스 연결 실패: " & Err.Description
    Response.End
End If

' ARS_DNIS로 ARS_TYPE 조회
qry = "SELECT ARS_TYPE FROM COMMON_DNIS_INFO WHERE ARS_DNIS = '" & Replace(ars_tel_no, "'", "''") & "' AND USE_YN = 'Y'"
Set rs = dbCon.Execute(qry)

If Err.Number <> 0 Then
    dbCon.Close
    Set dbCon = Nothing
    Response.Write ars_tel_no & "||9001|데이터베이스 조회 실패: " & Err.Description
    Response.End
End If

' 결과 처리
type_cd_list = ""

If rs.EOF Then
    ' 데이터 없음
    rs.Close
    Set rs = Nothing
    dbCon.Close
    Set dbCon = Nothing
    Response.Write ars_tel_no & "||9002|해당 전화번호를 찾을 수 없습니다."
    Response.End
End If

' 복수 결제유형 수집 (동일 DNIS에 여러 레코드가 있을 수 있음)
Dim typeDict, arsType
Set typeDict = Server.CreateObject("Scripting.Dictionary")

Do While Not rs.EOF
    arsType = Trim(rs("ARS_TYPE") & "")
    If arsType <> "" And Not typeDict.Exists(arsType) Then
        typeDict.Add arsType, 1
    End If
    rs.MoveNext
Loop

rs.Close
Set rs = Nothing
dbCon.Close
Set dbCon = Nothing

On Error GoTo 0

' 결제유형 목록 생성
If typeDict.Count = 0 Then
    ' 유형 미지정
    type_cd_list = "NDY"
Else
    Dim keys, i
    keys = typeDict.Keys
    For i = 0 To UBound(keys)
        If i > 0 Then
            type_cd_list = type_cd_list & ","
        End If
        type_cd_list = type_cd_list & keys(i)
    Next
End If

Set typeDict = Nothing

' ============================================
' 성공 응답
' ============================================

Response.Write ars_tel_no & "|" & type_cd_list & "|0000|결제유형획득에 성공하셨습니다. 여러개의 결제 유형이 배정된 경우, 페이엠솔루션과 협의하시기 바랍니다."
%>
