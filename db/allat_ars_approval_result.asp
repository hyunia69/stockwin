<%@ Language=VBScript %>
<%
    Const strConnect = "Provider=SQLOLEDB.1;Password=medi@ford;Persist Security Info=True;User ID=sa;Initial Catalog=arspg_web;Data Source=211.196.157.119"
    
    Dim dbCon, rs, mx_issue_no, shop_id, qry, shop_cnt, mx_cnt, approval_result, approval_message

    ' 입력값 수집
    mx_issue_no = Trim(Request.QueryString("order_no"))
    shop_id = Trim(Request.QueryString("shop_id"))

    ' 입력값 검증
    If mx_issue_no = "" Then
        Response.Write "0002|주문번호누락"
        Response.End
    End If
    If shop_id = "" Then
        Response.Write "0003|가맹점아이디누락"
        Response.End
    End If

    ' 데이터베이스 연결
    On Error Resume Next
    Set dbCon = Server.CreateObject("ADODB.Connection")
    dbCon.Open strConnect
    If Err.Number <> 0 Then
        Response.Write "0001|데이터베이스 연결 실패"
        Response.End
    End If
    On Error GoTo 0

    ' 가맹점 확인
    qry = "SELECT COUNT(shop_id) AS cnt FROM dbo.ALLAT_SHOP_ADMIN WHERE shop_id = '" & Replace(shop_id, "'", "''") & "'"
    Set rs = dbCon.Execute(qry)
    If rs.EOF Then
        shop_cnt = 0
    Else
        shop_cnt = rs("cnt")
    End If
    rs.Close
    Set rs = Nothing

    If shop_cnt = 0 Then
        Response.Write "0004|미등록가맹점"
        Response.End
    End If

    ' 주문번호 확인
    qry = "SELECT COUNT(mx_issue_no) AS cnt FROM dbo.ALLAT_SHOP_ORDER WHERE mx_id = '" & Replace(shop_id, "'", "''") & "' AND mx_issue_no = '" & Replace(mx_issue_no, "'", "''") & "'"
    Set rs = dbCon.Execute(qry)
    If rs.EOF Then
        mx_cnt = 0
    Else
        mx_cnt = rs("cnt")
    End If
    rs.Close
    Set rs = Nothing

    If mx_cnt = 0 Then
        Response.Write "0005|미등록주문"
        Response.End
    End If

    ' 승인내역 확인
    qry = "SELECT reply_code, reply_message FROM dbo.ALLAT_PAY_LOG WHERE mx_id = '" & Replace(shop_id, "'", "''") & "' AND mx_issue_no = '" & Replace(mx_issue_no, "'", "''") & "' ORDER BY reply_date DESC "
    Set rs = dbCon.Execute(qry)
    If rs.EOF Then
        approval_result = "9999"
        approval_message = "미승인"
    Else
        approval_result = "" & rs("reply_code")
        approval_message = "" & rs("reply_message")
    End If
    rs.Close
    Set rs = Nothing

    ' 결과 반환
    Response.Write approval_result & "|" & approval_message

    ' 리소스 정리
    dbCon.Close
    Set dbCon = Nothing
%>
