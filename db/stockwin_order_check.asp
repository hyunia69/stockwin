<!--METADATA TYPE="typelib" FILE="C:\program Files\Common Files\System\ado\msado15.dll"-->
<%
' ============================================
' StockWin 주문 데이터 조회 (테스트용)
' ============================================

Response.CharSet = "UTF-8"
Response.ContentType = "application/json; charset=UTF-8"

Dim strConnect
strConnect = "Provider=SQLOLEDB.1;Password=medi@ford;Persist Security Info=True;User ID=sa;Initial Catalog=arspg_web;Data Source=211.196.157.121"

Dim order_no
order_no = Request.QueryString("order_no")

If order_no = "" Then
    order_no = "TEST"
End If

Dim dbCon, rs, qry

On Error Resume Next

Set dbCon = Server.CreateObject("ADODB.Connection")
dbCon.Open strConnect

If Err.Number <> 0 Then
    Response.Write "{""error"": ""DB Connection Failed: " & Err.Description & """}"
    Response.End
End If

qry = "SELECT TOP 10 MX_ISSUE_NO, MX_NAME, MX_ID, ADMIN_ID, CC_NAME, CC_PORD_DESC, AMOUNT, PHONE_NO, REQUEST_TYPE, ITEM_CODE, REG_DATE, REPLY_CODE, REPLY_MESSAGE FROM ALLAT_SHOP_ORDER WHERE MX_ISSUE_NO LIKE '%" & Replace(order_no, "'", "''") & "%' ORDER BY REG_DATE DESC"

Set rs = dbCon.Execute(qry)

If Err.Number <> 0 Then
    dbCon.Close
    Set dbCon = Nothing
    Response.Write "{""error"": ""Query Failed: " & Err.Description & """}"
    Response.End
End If

Response.Write "{""results"": ["

Dim isFirst
isFirst = True

Do While Not rs.EOF
    If Not isFirst Then
        Response.Write ","
    End If
    isFirst = False

    Response.Write vbCrLf & "  {"
    Response.Write """MX_ISSUE_NO"": """ & rs("MX_ISSUE_NO") & ""","
    Response.Write """MX_NAME"": """ & rs("MX_NAME") & ""","
    Response.Write """MX_ID"": """ & rs("MX_ID") & ""","
    Response.Write """ADMIN_ID"": """ & rs("ADMIN_ID") & ""","
    Response.Write """CC_NAME"": """ & rs("CC_NAME") & ""","
    Response.Write """CC_PORD_DESC"": """ & rs("CC_PORD_DESC") & ""","
    Response.Write """AMOUNT"": " & rs("AMOUNT") & ","
    Response.Write """PHONE_NO"": """ & rs("PHONE_NO") & ""","
    Response.Write """REQUEST_TYPE"": """ & rs("REQUEST_TYPE") & ""","
    Response.Write """ITEM_CODE"": """ & rs("ITEM_CODE") & ""","
    Response.Write """REG_DATE"": """ & rs("REG_DATE") & ""","
    Response.Write """REPLY_CODE"": """ & rs("REPLY_CODE") & ""","
    Response.Write """REPLY_MESSAGE"": """ & rs("REPLY_MESSAGE") & """"
    Response.Write "}"

    rs.MoveNext
Loop

rs.Close
Set rs = Nothing
dbCon.Close
Set dbCon = Nothing

Response.Write vbCrLf & "]}"
%>
