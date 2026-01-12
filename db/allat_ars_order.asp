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
  
  strMode     = trim(Request.Form("mode"))
  mx_issue_no = trim(request.Form("order_no"))  '//가맹점 거래번호
  shop_id     = trim(request.Form("shop_id"))      '//가맹점 아이디
  req_type    = trim(request.Form("request_type"))
  alert_show  = trim(request.Form("alert_show"))
  verify_num     = trim(request.Form("verify_num"))       '//인증번호
  
  cc_name     = trim(request.Form("cc_name"))      '//고객명         
  cc_pord_desc  = trim(request.Form("cc_pord_desc")) '//상품명
  amount      = trim(request.Form("amount"))       '//결제금액
  phone_no    = trim(request.Form("phone_no"))     '//고객 핸드폰    

  
  logTime = FormatDateTime(Now(), 0)
  sysDate = FormatDateTime(Now(), 2)
  
  FileName = ".\logs\"& sysDate & ".log"
  
  Const ForReading = 1, ForWriting = 2, ForAppending = 8

  dim fs,logFile
  set fs=Server.CreateObject("Scripting.FileSystemObject")
  set f=fs.OpenTextFile(Server.MapPath(FileName),ForAppending,true)
  
  
  logCont = logTime & " strMode(" &  strMode & "), mx_issue_no(" & mx_issue_no &  "), shop_id(" & shop_id & "), req_type(" & req_type & ") , alert_show (" & alert_show &"), verify_num (" & verify_num &"), cc_name (" & cc_name &"), cc_pord_desc (" & cc_pord_desc &"), amount (" & amount &"), phone_no (" & phone_no &")"
  

  f.WriteLine(logCont)
  
  
  If strMode <> "ars_data_add" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0001|데이터구분누락"
    response.end
  End if
  
  If mx_issue_no = "" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0002|주문번호누락"
    response.end
  End if
  
  If shop_id = "" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0003|가맹점아이디누락"
    response.end
  End if
  
  If req_type = "" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0004|ARS구분누락"
    response.end
  End if

'  If verify_num = "" Then
'    response.write mx_issue_no & "|" & phone_no & "|" & "0005|인증번호누락"
'    response.end
'  End if
  
  If cc_name = "" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0006|고객명누락"
    response.end
  End if
  
  If cc_pord_desc = "" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0007|상품명누락"
    response.end
  End if
  
  If amount = "" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0008|결제금액누락"
    response.end
  End if
  
  If phone_no = "" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0009|고객핸드폰누락"
    response.end
  End if

    
  '//상점 정보 가져오기
  set cmd = Server.CreateObject("ADODB.Command")
  with cmd
      .ActiveConnection = strConnect
      .CommandType = adCmdStoredProc
      .CommandTimeout = 60
      .CommandText = "dbo.sp_getAllatShopInfo"
      .Parameters.Append .CreateParameter("@SHOP_ID", adVarChar, adParamInput, 20, shop_id)
      set rs = .Execute
  end with
  set cmd = nothing
  
  If Not rs.EOF Then
    mx_name  = trim(rs("shop_name")) '//가맹점명
    mx_id    = trim(rs("shop_id"))   '//가맹점 아이디
    mx_opt   = trim(rs("shop_key"))  '//가맹점 접근키
    ars_dnis = trim(rs("ars_dnis"))  '//DNIS 번호
  Else
    response.write mx_issue_no & "|" & phone_no & "|" & "0010|가맹점아이디불일치"
    response.end
  End if
  rs.close
  set rs = nothing
  
  '//주문번호 중복 확인
  Set dbCon = Server.CreateObject("ADODB.Connection")
  dbCon.Open strConnect
  qry = "SELECT count(mx_issue_no) cnt  FROM dbo.ALLAT_SHOP_ORDER where mx_id = '"& shop_id &"' and mx_issue_no = '"& mx_issue_no &"'"
  Set rs = dbCon.Execute(qry)
  mx_cnt = rs("cnt")
  rs.close
  set rs = nothing

  If mx_cnt = 0 then
    If req_type = "SMS" Then
      If ars_dnis <> "" Then
        callback_no = "02-3490-" & ars_dnis
      Else
        callback_no = "02-3490-4491"
      End if

     'maxcode  = verify_num
    
      'If verify_num = "" Then
    
        '인증번호 구하기
        set cmd = Server.CreateObject("ADODB.Command")
        with cmd
          .ActiveConnection = strConnect
          .CommandType = adCmdStoredProc
          .CommandTimeout = 60
          .CommandText = "sp_getAllatAuthNo"
          set arsRs = .Execute
        end with
        set cmd = nothing
        If IsNull(arsRs(0)) then
          maxcode = "000100"
        Else
          for i=1 to 6-len(arsRs(0))
            maxcode = maxcode & "0"
          next
          maxcode = maxcode & arsRs(0)
        End if
        arsRs.close
        Set arsRs = nothing
        
      'End if
     
      smsMsg = mx_name & " 결제 주문인증번호는[" & maxcode & "]입니다 " 
      smsMsg = smsMsg & callback_no & " 연결해주십시요"
       
      Set smsRs = Server.CreateObject("ADODB.Recordset")
      with smsRs
        .Open "em_smt_tran", strConnectSMS, adOpenDynamic, adLockOptimistic
        .AddNew
        .Fields("mt_refkey")       = mx_id & "@" &  mx_issue_no
        .Fields("rs_id")           = "ALLAT"
        .Fields("date_client_req") = now()
        .Fields("content")         = smsMsg
        .Fields("callback")        = callback_no
        .Fields("service_type")    = "0"
        .Fields("broadcast_yn")    = "N"
        .Fields("msg_status")      = "1"
        .Fields("recipient_num")   = Trim(Request.Form("phone_no"))
        .Update
      End with
      smsRs.Close
      Set smsRs = nothing
    End if

     
    '//주문등록  
    Set adoRs = Server.CreateObject("ADODB.Recordset")
    with adoRs
      .Open "ALLAT_SHOP_ORDER", strConnect, adOpenDynamic, adLockOptimistic
      .AddNew
      .Fields("mx_issue_no")  = mx_issue_no                        '//가맹점 거래번호
      .Fields("mx_name")      = mx_name                            '//가맹점 이름      
      .Fields("mx_id")        = mx_id                              '//가맹점 아이디     
      .Fields("mx_opt")       = mx_opt                             '//가맹점 접근키
      .Fields("admin_id")     = shop_id                            '//관리자 아이디
      .Fields("admin_name")   = "시스템"                           '//관리자명
      .Fields("cc_name")      = cc_name                            '//고객명         
      .Fields("cc_pord_desc") = cc_pord_desc                       '//상품명         
      .Fields("cc_email")     = trim(request.Form("cc_email"))     '//고객 이메일      
      .Fields("amount")       = amount                             '//결제금액        
      .Fields("phone_no")     = phone_no                           '//고객 핸드폰      
      .Fields("payment_code") = "0"                                '//결제유무 (0:미결제, 1:결제완료, 2:취소완료)
      .Fields("request_type") = req_type                           '//접수구분 (ARS:호전환, SMS:SMS)
    If maxcode <> "" Then
      .Fields("auth_no")  = maxcode
    End if
      .Update
    End with
    adoRs.Close
    Set adoRs = nothing
    dbCon.close
    
    If alert_show = "Y" Then
      response.write mx_issue_no & "|" & phone_no & "|" & "0000|등록성공"
      response.end
    End if
    response.write mx_issue_no & "|" & phone_no & "|" & "0000|등록성공"
    response.end
  Else
    response.write mx_issue_no & "|" & phone_no & "|" & "0011|거래번호오류"
    response.end
  End if
%>
