<!--METADATA TYPE="typelib" FILE="C:\program Files\Common Files\System\ado\msado15.dll"-->
<%
	Dim strConnect, strConnectSMS
	strConnect="Provider=SQLOLEDB.1;Password=medi@ford;Persist Security Info=True;User ID=sa;Initial Catalog=arspg_web;Data Source=211.196.157.119"
	strConnectSMS="Provider=SQLOLEDB.1;Password=imds@00;Persist Security Info=True;User ID=imds;Initial Catalog=imds;Data Source=211.196.157.119"
  Dim mx_cnt
  
  Function eregi_replace(pattern, replace, text)
    Dim eregObj:
    Set eregObj = New RegExp:
    eregObj.Pattern = pattern: '//패턴 설정
    eregObj.IgnoreCase = True: '//대소문자 구분 여부
    eregObj.Global = True: '//전체 문서에서 검색
    eregi_replace = eregObj.Replace(text, replace): '//Replace String
  End Function
  
  mx_issue_no = trim(request.Form("order_no"))  '//가맹점 거래번호
  shop_id     = trim(request.Form("shop_id"))      '//가맹점 아이디

  '//주문번호 확인
  Set dbCon = Server.CreateObject("ADODB.Connection")
  dbCon.Open strConnect
  qry = "SELECT count(mx_issue_no) cnt  FROM dbo.ALLAT_SHOP_ORDER where mx_id = '"& shop_id &"' and mx_issue_no = '"& mx_issue_no &"'"
  Set rs = dbCon.Execute(qry)
  mx_cnt = rs("cnt")
  rs.close
  set rs = nothing
  dbCon.close
  set dbCon = nothing   

  '//결제 내역  확인
  Set dbCon = Server.CreateObject("ADODB.Connection")
  dbCon.Open strConnect
  qry = "SELECT count(mx_issue_no) pcnt  FROM dbo.ALLAT_SHOP_ORDER where mx_id = '"& shop_id &"' and mx_issue_no = '"& mx_issue_no &"' and payment_code = '0'"
  Set rs = dbCon.Execute(qry)
  payment_cnt = rs("pcnt")
  rs.close
  set rs = nothing
  dbCon.close
  set dbCon = nothing   

    
  If mx_cnt = 0 then
    response.write mx_issue_no & "|" & mx_id & "|" & "0011|미등록거래"
    response.end
  Else

    If payment_cnt = 0 then
      response.write mx_issue_no & "|" & mx_id & "|" & "0012|결제완료거래"
      response.end
    Else      
      '//주문 삭제 
      Set dbCon = Server.CreateObject("ADODB.Connection")
      dbCon.Open strConnect
      sql = "DELETE FROM dbo.allat_shop_order WHERE mx_issue_no = '" & mx_issue_no & "' AND mx_id = '" & shop_id & "'"
      dbCon.Execute(sql)
      dbCon.close
      set dbCon = nothing   
    
      response.write mx_issue_no & "|" & mx_id & "|" & "0000|삭제성공"
      response.end
    End if 
  End if
%>
