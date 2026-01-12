<!--METADATA TYPE="typelib" FILE="C:\program Files\Common Files\System\ado\msado15.dll"-->
<meta http-equiv="Content-Type" content="text/html; charset=euc-kr" />
<%
	Dim strConnect, strConnectSMS
	strConnect="Provider=SQLOLEDB.1;Password=medi@ford;Persist Security Info=True;User ID=sa;Initial Catalog=arspg_web;Data Source=211.196.157.119"
	strConnectSMS="Provider=SQLOLEDB.1;Password=imds@00;Persist Security Info=True;User ID=imds;Initial Catalog=imds;Data Source=211.196.157.121"
  
  Function eregi_replace(pattern, replace, text)
    Dim eregObj:
    Set eregObj = New RegExp:
    eregObj.Pattern = pattern: '//패턴 설정
    eregObj.IgnoreCase = True: '//대소문자 구분 여부
    eregObj.Global = True: '//전체 문서에서 검색
    eregi_replace = eregObj.Replace(text, replace): '//Replace String
  End Function
  

  mx_issue_no = trim(Request.QueryString("order_no"))  '//가맹점 거래번호
  shop_id     = trim(Request.QueryString("shop_id"))      '//가맹점 아이디
 

  
  If mx_issue_no = "" Then
    response.write"0002|주문번호누락"
    response.end
  End if
  
  If shop_id = "" Then
    response.write"0003|가맹점아이디누락"
    response.end
  End if




  '//가맹점 확인
  Set dbCon = Server.CreateObject("ADODB.Connection")
  dbCon.Open strConnect
  qry = "SELECT count(shop_id) cnt  FROM dbo.ALLAT_SHOP_ADMIN where shop_id = '"& shop_id &"'"
  Set rs = dbCon.Execute(qry)
  shop_cnt = rs("cnt")
  If shop_cnt = 0 Then
    response.write mx_issue_no & "|" & "0005|미등록가맹점"
    response.end
  End if  

  
  '//주문번호 확인
  Set dbCon = Server.CreateObject("ADODB.Connection")
  dbCon.Open strConnect
  qry = "SELECT count(mx_issue_no) cnt  FROM dbo.ALLAT_SHOP_ORDER where mx_id = '"& shop_id &"' and mx_issue_no = '"& mx_issue_no &"'"
  Set rs = dbCon.Execute(qry)
  mx_cnt = rs("cnt")
  If mx_cnt = 0 Then
    response.write mx_issue_no & "|" & "0005|미등록주문"
    response.end
  End if  
  
 
  '//주문번호 중복 확인
  Set dbCon = Server.CreateObject("ADODB.Connection")
  dbCon.Open strConnect
  qry = "SELECT reply_code, reply_message  FROM dbo.ALLAT_PAY_LOG where mx_id = '"& shop_id &"' and mx_issue_no = '"& mx_issue_no &"'"
  Set rs = dbCon.Execute(qry)



If rs.EOF Then
    approval_result = ""
    approval_message = ""
Else
    approval_result = rs("reply_code")
    approval_message = rs("reply_message")
End If

  rs.close
  set rs = nothing

  If approval_result = "" Then
     response.write mx_issue_no & "|" & "9999|미승인"
      response.end
  Else 
      response.write mx_issue_no & "|" & approval_result& "|" & approval_message
      response.end
  End if

%>
