<!--METADATA TYPE="typelib" FILE="C:\program Files\Common Files\System\ado\msado15.dll"-->
<meta http-equiv="Content-Type" content="text/html; charset=euc-kr" />
<%
	Dim strConnect, strConnectSMS
	strConnect="Provider=SQLOLEDB.1;Password=medi@ford;Persist Security Info=True;User ID=sa;Initial Catalog=arspg_web;Data Source=211.196.157.119"
	strConnectSMS="Provider=SQLOLEDB.1;Password=imds@00;Persist Security Info=True;User ID=imds;Initial Catalog=imds;Data Source=211.196.157.121"
  
    
'  Dim encodeTest : encodeTest = Server.URLencode("한글")
'  Dim decodeTest : decodeTest = URLDecode(encodeTest)
 
'  Response.Write encodeTest + "=" + decodeTest는 다음의 Response.Write에 영향을 준다....
' 즉 다음에 나오는 Response.Write 앞에 합치 되어짐을 유의 하라!
' 새로운 명령이라도 + 나 & 하지 않더라도 합치 된다.
 
  '### Decode 함수 시작 ###
 'EUC-KR 용(??)
  Function URLDecode(Expression)
   Dim strSource, strTemp, strResult, strchr
   Dim lngPos, AddNum, IFKor
   strSource = Replace(Expression, "+", " ")
   For lngPos = 1 To Len(strSource)
    AddNum = 2
    strTemp = Mid(strSource, lngPos, 1)
    If strTemp = "%" Then
     If lngPos + AddNum < Len(strSource) + 1 Then
      strchr = CInt("&H" & Mid(strSource, lngPos + 1, AddNum))
      If strchr > 130 Then 
       AddNum = 5
       IFKor  = Mid(strSource, lngPos + 1, AddNum)
       IFKor  = Replace(IFKor, "%", "")
       strchr = CInt("&H" & IFKor )
      End If
      strResult = strResult & Chr(strchr)
      lngPos = lngPos + AddNum
     End If
    Else
     strResult = strResult & strTemp
    End If
   Next
   URLDecode = strResult
  End Function  
  
  'Encoding된 파라미터를 DecoDing 해준다.
Function FnURLDecode(sStr)

    Dim sRet, reEncode, sChar
    Dim i

    If isnull(sStr) Then
        sStr = ""
    Else
        Set reEncode = New RegExp
            reEncode.IgnoreCase = True
            reEncode.Pattern = "^%[0-9a-f][0-9a-f]$"
            sStr = Replace(sStr, "+", " ")
            sRet = ""
            
            For i = 1 To Len(sStr)
                sChar = Mid(sStr, i, 3)
                If reEncode.Test(sChar) Then
                    If CInt("&H" & Mid(sStr, i + 1, 2)) < 128 Then
                        sRet = sRet & Chr(CInt("&H" & Mid(sStr, i + 1, 2)))
                        i = i + 2
                    Elseif mid(sStr, i+3, 1) ="%" Then
                        sRet = sRet & Chr(CInt("&H" & Mid(sStr, i + 1, 2) & Mid(sStr, i + 4, 2)))
                        i = i + 5 
                    Else 
                        sRet = sRet & Chr(CInt("&H" & Mid(sStr, i + 1, 2) & "00") + asc(mid(sStr,i+3,1)))
                        i = i + 3
                        '이부분이 중요하다.
                        '기존 urldecode함수가 몇 몇 글자들에서 에러를 내는 이유는 3바이트로 인코딩 되어있는 부분이 있기 때문. ascII로 변형되어 인코딩 된 부분이 존재한다.
                    End If
                Else
                    sRet = sRet & Mid(sStr, i, 1)
                End If
            Next
    End If
    FnURLDecode = sRet
End Function

  'UTF-8 용
  Function UrlDecode_GBToUtf8(ByVal str) 
    Dim B,ub    ''中文字的Unicode?(2字?) 
    Dim UtfB    ''Utf-8??字? 
    Dim UtfB1, UtfB2, UtfB3 ''Utf-8?的三?字? 
    Dim i, n, s 
    n=0 
    ub=0 
    For i = 1 To Len(str) 
        B=Mid(str, i, 1) 
        Select Case B 
            Case "+"
                s=s & " "
            Case "%"
                ub=Mid(str, i + 1, 2) 
                UtfB = CInt("&H" & ub) 
                If UtfB<128 Then
                    i=i+2 
                    s=s & ChrW(UtfB) 
                Else
                    UtfB1=(UtfB And &H0F) * &H1000    ''取第1?Utf-8字?的二?制后4位 
                    UtfB2=(CInt("&H" & Mid(str, i + 4, 2)) And &H3F) * &H40        ''取第2?Utf-8字?的二?制后6位 
                    UtfB3=CInt("&H" & Mid(str, i + 7, 2)) And &H3F        ''取第3?Utf-8字?的二?制后6位 
                    s=s & ChrW(UtfB1 Or UtfB2 Or UtfB3) 
                    i=i+8 
                End If 
            Case Else    ''Ascii? 
                s=s & B 
        End Select
    Next
    UrlDecode_GBToUtf8 = s 
End Function

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
  
  cc_name     = URLDecode(trim(request.Form("cc_name")))     '//고객명         
  cc_pord_desc  = URLDecode(trim(request.Form("cc_pord_desc"))) '//상품명
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

  If req_type = "SMS" and verify_num = "" Then
    response.write mx_issue_no & "|" & phone_no & "|" & "0005|인증번호누락"
    response.end
  End if
  
  ' 2016.09.08
  ' 고객명을 필수 항목 아님
  ' 한국 경제 손상민 팀장 정의 함
  ' 어라!!! 이걸 어째 디비 테이블은 이것이 필수 항목이라 어쩔 도리 없이 쓰기로 함
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
    response.write mx_issue_no & "|" & phone_no & "|" & "0011|거래번호중복오류"
    response.end
  End if
%>
