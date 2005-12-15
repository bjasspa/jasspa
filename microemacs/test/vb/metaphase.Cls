VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "Metaphase"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

' Davey Stuff

Public ObjectType As String
Public PartName As String
Public AssemblyOutput As String
Public PartOutput As String
Public PartOBID As String


Public CacheFile As Boolean

' Davey - End


Public id

Private ResultBuf As Buffer

Private DependencyLog As Buffer
Private DependencyErrorOccurred As Boolean

Private HistoryLog As Buffer

Public server As String
Public db As String

Public UserName As String
Public Password As String

Public ClientIP As String
Public MaxJobTime

Public gfsScriptName As String
Public gedScriptName As String
Public gflScriptName As String
Public wusScriptName As String
Public gpsScriptName As String
Public mdlScriptName As String

Public MaxDepth As Long

Public IncludeProperties As Boolean
Public IncludeStatus As Boolean
Public IncludeRevHistory As Boolean
Public IncludeDependencies As Boolean
Public IncludeFiles As Boolean

Public NameOverride As String
Public ShowBothInTree As Boolean

Public DirectAccessEnabled As Boolean
Public DataBaseRootPath As String

Private edID As Long
Private Depth As Long

Private Components As Collection
Private PropTypeIDs As Collection

Private LastInstName As String

Private Const PROPSET_PROPERTIES = "Properties"
Private Const PROPSET_STATUS = "Status"
Private Const PROPSET_REVHISTORY = "History"
Private Const PROPSET_DEPENDENCIES = "Dependencies"
Private Const PROPSET_FILES = "Files"
Private CompNameOverride As String

Private CurrentVersion As String

Private MP_LINE_DELIM As String

Public TimeOutTime

Private AlreadyFetched As Collection
Private AlreadyFetchedVersions As Collection
Private Structure As Object

Private DepLogDepth As Long

Private Function GetAnswer(Question As String, CacheExt As String, buf As pString) As Boolean
  
  Dim Entry As Object
  Dim OK As Boolean
  Dim Job As New QueryJob
  
  If Timer > TimeOutTime Then
    If Timer - TimeOutTime < 50000 Then
      Exit Function
    ElseIf Timer - TimeOutTime < 50000 Then
      Exit Function
    Else
      Exit Function
    End If
  End If
  
  Job.id = id
  
  Job.ClientIP = ClientIP
  Job.UserName = UserName
  Job.Password = Password
  Job.server = server
  Job.db = db
  Job.CacheExt = CacheExt
  Job.Question = Question
    
  lPrint "GetAnswer: " & Job.Question
  Agent.ServiceJob Job
    
  If Job.Status = "" Then
    GetAnswer = True
    Set buf = Job.Answer
  Else
    CGIPrint Job.Status
  End If
    
End Function
Private Function CGIPrepareString(RecSpec As String) As String
  
  Dim URL As String

  URL = SandR(RecSpec, "\", "/")
  URL = SandR(URL, ";", "%3A")
  URL = SandR(URL, "@", "%40")
  URL = SandR(URL, "&", "%26")
  URL = SandR(URL, "#", "%23")
  
  CGIPrepareString = URL

End Function

Private Sub DefinePropType(B, s As String)
  
  Dim i As Long
  
  If B Then
    i = PropTypeIDs.count + 1
    PropTypeIDs.Add i, s
    CGIPrint "T " + CStr(i) + " " + s
  End If
  
End Sub
  
Private Function GetCompID(CompName As String, CompID As Long) As Long
  
  Dim id As Long
  
  On Error Resume Next
  
  id = Components(CompName)
  If Err Then
    Components.Add CompID, CompName
  Else
    GetCompID = id
  End If
  On Error GoTo 0
  
End Function

Private Sub CGIPrint(s As String)
  
  Dim n As Long
  
  If ResultBuf Is Nothing Then
    Set ResultBuf = New Buffer
  End If
  
  n = ResultBuf.Append(s + vbCrLf)
  If Not Agent Is Nothing Then
    Agent.BytesOut = Agent.BytesOut + n
  End If
  
End Sub

Private Sub CGISend(s As String)
  
  Dim n As Long
  
  If ResultBuf Is Nothing Then
    Set ResultBuf = New Buffer
  End If
  
  n = ResultBuf.Append(s)
  If Not Agent Is Nothing Then
    Agent.BytesOut = Agent.BytesOut + n
  End If
  
End Sub

Private Function MakeObjectURL(Path As String, ObjectName As String, Version As String, Revision As String, CGIName) As String
  
  MakeObjectURL = CGIName & "?" & FOLDER_EQUAL & CGIPrepareString(Path) & VERSION_EQUAL & CGIPrepareString(Version) & REVISION_EQUAL & CGIPrepareString(Revision) & OBJECT_EQUAL & CGIPrepareString(ObjectName)
  
End Function

Private Function nid() As Long
  
  edID = edID + 1
  nid = edID
  
End Function
Public Function Terminate() As String
  
  Set ResultBuf = Nothing
  Agent.Terminate
  
End Function

Private Sub cgiError(s As String)
  
  CGIPrint "Content-type: text/html"
  CGIPrint ""
  CGIPrint s
  
End Sub

Public Property Get Result() As String
  
  If Not ResultBuf Is Nothing Then
    Result = ResultBuf.Contents
  End If
  
End Property

Public Property Let Result(s As String)

  Set ResultBuf = New Buffer
  
  ResultBuf.Append s
  
End Property

Private Function URLDecode(s As String) As String

  Dim i As Integer
  Dim t As String
  Dim C As String
  
  i = 1
  While i <= Len(s)
    Select Case Mid(s, i, 1)
      Case "%"
        C = Chr(Val("&H" + Mid(s, i + 1, 2)))
        i = i + 2
      Case Else
        C = Mid(s, i, 1)
    End Select
    t = t + C
    i = i + 1
  Wend
  
  URLDecode = t

End Function

Private Sub Class_Initialize()
  
  MP_LINE_DELIM = vbLf
  
  ' Read the inifiles
    
  ReadAgentIniFile
 
  Randomize Timer
  
End Sub

Private Sub DepLogMsg(Msg As String)
  
  Dim s As String
  
  If DependencyLog Is Nothing Then
    Set DependencyLog = New Buffer
  End If
  
  s = Space(DepLogDepth) & Msg & vbCrLf
  DependencyLog.Append s
  
  If Not LogForm Is Nothing Then
    LogForm.DependencyMsg s
  End If
  
End Sub
Private Sub HistLogMsg(Msg As String)

  Dim s As String
  
  If HistoryLog Is Nothing Then
    Set HistoryLog = New Buffer
  End If
  
  s = Msg & vbCrLf
  HistoryLog.Append s
  
  If Not LogForm Is Nothing Then
    LogForm.HistoryMsg s
  End If
  
End Sub
Private Sub DepLogError(Msg As String, Path As String, ObjectName As String, Version As String, Revsision As String)
  
  Dim t As Long
  
  DependencyErrorOccurred = True
  
  t = DepLogDepth
  DepLogDepth = 0
  
  DepLogMsg ""
  DepLogMsg Msg
  DepLogMsg server & " " & db & " " & Path & " " & ObjectName
  DepLogMsg Result
  
  DepLogDepth = t
  
End Sub

Public Sub PostLogFile(Section As String, Key As String, Log As Buffer)
  
  Agent.PostLogFile Section, Key, Log

End Sub

Public Sub TellSomeOne(Msg As String, Section As String, Key As String)
 
   Agent.TellSomeOne Msg, Section, Key
   
End Sub

' Davey's Functions

'***************************************************************************
' Logon
' - Logs on to a MP session
'
' Inputs
' - Username
' - Password
'
' Output
' - Boolean
'***************************************************************************

Public Function Logon(UserName As String, Password As String) As Boolean

  Dim buf As pString
  Dim Query As String
  Dim Result As String

  Set ResultBuf = New Buffer

  Query = "lon " & UserName & " " & Password

  lPrint "Login on: " & Query

  If GetAnswer(Query, "lon.txt", buf) Then

    If buf.n > 0 Then
      Result = Trimmer(GetNext(buf.s, OA_PROMPT))
      CGISend Result
    End If
    
    Logon = True
    
  Else
    lPrint "Logon fails"
    
  End If
             
End Function

'***************************************************************************
' QueryMP
' - Querys MP for the available parts
'
' Input
' - ObjectType
' - PartName
' - AssemblyOutput - Format of Assembly Output
' - PartOutput     - Format of Part Output
'
' Output
' - Boolean
'***************************************************************************

Public Function QueryMP(ObjectType As String, PartName As String, AssemblyOutput As String, PartOutput As String) As Boolean

  Dim buf As pString
  Dim Query As String
  Dim Result As String
  Dim OneLine As New pString
  Dim OutputLine As String
  
  Dim ItemType As String
  Dim ItemName As String
  Dim OBID As String
  Dim ProjectName As String
  Dim Version As String
  Dim Creator As String
  
  Set ResultBuf = New Buffer
  ResultBuf.ChunkSize = 65536

  Query = "qry " & ObjectType & " " & PartName

  lPrint "Sending: " & Query

  If GetAnswer(Query, "qry.txt", buf) Then
    
    ' Now we have to decode the result and format it so that
    ' the browser can display it properly
    
    ' The format of the string should be
    ' ObjectType:ObjectName:OBID:ProjectName:Version:Creator
    
    While buf.n > 0
      
      ' Delete the new line character from the end of the string
      OneLine.s = Trimmer(buf.GetNext(MP_LINE_DELIM))
      
      ' Get the type of object that we are dealing with
      ItemType = OneLine.GetNext(":")
      
      ' Decided what to do for different types of data
      'If ItemType = "Assembly" Then
      
      OutputLine = AssemblyOutput
      
      ' Exact the data that we are going to insert into the HTML
      ItemName = OneLine.GetNext(":")
      OBID = OneLine.GetNext(":")
      ProjectName = OneLine.GetNext(":")
      Version = OneLine.GetNext(":")
      Creator = OneLine.GetNext(":")
      
      ' Now insert it into the HTML page
      OutputLine = SandR(OutputLine, "{name}", ItemName)
      OutputLine = SandR(OutputLine, "{obid}", OBID)
      OutputLine = SandR(OutputLine, "{name}", ItemName)
      OutputLine = SandR(OutputLine, "{version}", Version)
      OutputLine = SandR(OutputLine, "{creator}", Creator)
      
      ' Send it out
      CGIPrint OutputLine
      
      'ElseIf ItemType = "Component" Then
      
      ' What to do about componets
      
      'End If
      
    Wend
    
    QueryMP = True
    
  Else
    
    lPrint "QueryMP fails"
    
  End If
  
End Function

'***************************************************************************
' GenerateProductStructure
' - Generates the product structure from MP
'
' Input
' - obid - This uniquely identifies any object within MP
'
' Output
' - Boolean
'***************************************************************************

Public Function GenerateProductStructure(OBID As String) As Boolean

  Dim buf As pString
  Dim Query As String
  Dim Result As String

  Dim OneLine As New pString
  Dim OutputLine As String
  
  Query = "gps " & OBID

  lPrint "Sending: " & Query

  If GetAnswer(Query, "gps.txt", buf) Then
    
    If buf.n > 0 Then
      CGISend buf.s
    End If
    
    GenerateProductStructure = True
    
  Else

    lPrint "GenerateProductStructure fails"
  
  End If

End Function

'***************************************************************************
' GetFile
' - Gets a file from MP
'
' Input
' - obid - This uniquely identifies any object within MP
'
' Output
' - Boolean
'
' Note
' - Because we are not using an agent to FTP over to another machine
' to get any files, rather, we use the MP interface to extract any files
' that we want then this GetFile routine must check to see if the file is in cache
' ie it must do the same task as an Agent.service job does
'***************************************************************************

Public Function GetFile(OBID As String, PartName As String, DestPath As String) As Boolean
  
  Dim Job As New FetchJob
  Dim buf As pString
  Dim Query As String
  Dim TransferPath As String
  Dim CacheSave As Boolean
  
  ' For checking in Cache
  Dim Entry As Object
  Dim Cache As Object
  
  Set ResultBuf = New Buffer
  
  Dim EDR As Object
  Dim CadAdapter As Object
  
  Dim i
  Dim application As String, ext As String
  Dim convert As Boolean
  Dim TimeStamp
  Dim fSpec As String
  Dim FileExtension As String
  Dim DestFile As String
  Dim retfile As String
  
  Dim Result
  
  ' Get a handle to the EDR
  Set EDR = CreateObject("EDR.Proxy")
  EDR.Tool id, "Metaphase", MAGIC_KEY
  EDR.TransferPath = TransferPath
  EDR.MaxJobTime = MaxJobTime
  EDR.db = "Metaphase"
  EDR.Project = "Division"
  
  Query = "gfl " & OBID & " " & DestPath
  
  lPrint "Sending: " & Query
  
  
  
  ' The Outer if - statement
  TimeStamp = UpdateTimeFile(OBID)
  
  If TimeStamp <> "" Then
    
    ' Check the EDR for the object
    
    If EDR.GetObjectPath(OBID, PartName, TimeStamp) Then
      fSpec = EDR.Result
    End If
    
    If fSpec <> "" Then
      
      ' We have the file already
      
      Result = fSpec
      GetFile = True
      
    Else
      
      ' We need to get the file
      
      If GetAnswer(Query, "qry.txt", buf) Then
        
        Result = buf.s
        
        ' We have extracted the file and we now need to consider if
        ' we need to do a live conversion
        
        ' Do we need to do a conversion?
        
        FileExtension = GetLast(PartName, ".")
        
        If FileExtension <> "ol" Then
          
          ' Need to identify what type of file we have
          For i = 0 To UBound(CADEXTS, 1)
            
            If FileExtension = GetLast(CADEXTS(i, 1), ".") Then
              
              application = CADEXTS(i, 0)
              ext = CADEXTS(i, 1)
              convert = True
            End If
            
          Next
          
          If convert Then
            
            ' We need to convert the file
            
            DestFile = Result
            
            Set CadAdapter = CreateObject("olObjectAdapter.CAD")
            CadAdapter.MaxJobTime = MaxJobTime
            CadAdapter.Tool id, "Metaphase", MAGIC_KEY
            
            fSpec = CadAdapter.ConvertCadFile(application & ext, DestFile, "", "", "")
            
            ' Delete the original CAD model out of the cache
            
            DeleteFile DestFile
            
            ' Check to see if we have any errors
            
            If InStr(fSpec, "Error") > 0 Then
              
              ' Bummer !
              
              Result = fSpec & "CadAdapter.ConvertCadFile failed"
              GetFile = False
              
            Else
              
              ' Insert the object into the EDR
              
              retfile = EDR.StoreNewObject("Metaphase", OBID, PartName, "", "", fSpec, TimeStamp, "")
              GetFile = retfile
              
              fSpec = EDR.Result
              
            End If
            
          Else
            
            fSpec = Result
            
          End If
          
        Else
          
          ' The file is an ol file, do nothing
          
          fSpec = Result
          
        End If
        
      End If
      
    End If
    
  End If
  GetFile = True
  
  If fSpec <> "" Then
    CGISend fSpec
  End If
  
  
  '  Else
  '
  '    lPrint "GetFile fails"
  '
  '  End If
  
End Function

'***************************************************************************
' WUQuery
' - Return the assemblies where the input assembly is being used
'
' Input
' - obid           - This uniquely identifies any object within MP
' - AssemblyOutput - Format of Assembly Output
'
' Output
' - Boolean
'***************************************************************************

Public Function WUSQuery(OBID As String, AssemblyOutput As String) As Boolean

  Dim buf As pString
  Dim OneLine As New pString
  
  Dim ObjOBID As String
  Dim ItemName As String
  Dim Project As String
  Dim Revision As String
  Dim Creator As String
  
  Dim OutputLine As String
  Dim Query As String
  
  Dim Output As String
  
  Set ResultBuf = New Buffer
  ResultBuf.ChunkSize = 65536
        
  Query = "wus " & OBID
  
  lPrint "WUSQuery: " & Query
  
  If GetAnswer(Query, "wus.txt", buf) Then
    
    While buf.n > 0
      
      ' Delete the new line character from the end of the string
       OneLine.s = buf.GetNext(MP_LINE_DELIM)
       
       ' Extract the data that we are going to insert into into the HTML
       ObjOBID = OneLine.GetNext(":")
       ItemName = OneLine.GetNext(":")
       Project = OneLine.GetNext(":")
       Revision = OneLine.GetNext(":")
       Creator = OneLine.GetNext(":")
       
       ' Get the format for the line
       OutputLine = AssemblyOutput
       
       ' Now insert it into the HTML page
       OutputLine = SandR(OutputLine, "{obid}", ObjOBID)
       OutputLine = SandR(OutputLine, "{name}", ItemName)
       OutputLine = SandR(OutputLine, "{projectname}", Project)
       OutputLine = SandR(OutputLine, "{revision}", Revision)
       OutputLine = SandR(OutputLine, "{creator}", Creator)
       CGIPrint OutputLine
     
     Wend
  
     WUSQuery = True
    
   Else
   
    WUSQuery = False
     lPrint "WUSQuery fails"
  
   End If
End Function

'***************************************************************************
' UpdateTime
' - Return last update time for a particular part
'
' Input
' - obid - This uniquely identifies any object within MP
'
' Output
' - Boolean
'***************************************************************************

Public Function UpdateTime(OBID As String, partType As String) As Boolean

  Dim Job As New FetchJob
  Dim buf As pString
  Dim Query As String
  Dim TransferPath As String
  Dim CacheSave As Boolean
     
  ' For checking in Cache
  Dim Entry As Object
  Dim Cache As Object
    
  Set ResultBuf = New Buffer
    
  Query = "upt " & OBID & " " & partType
  
  lPrint "Sending: " & Query
  
  If GetAnswer(Query, "qry.txt", buf) Then
    
    If buf.n > 0 Then
      CGISend buf.s
    End If
    
    UpdateTime = True
    
  Else

    lPrint "UpdateTime fails"
  
  End If

End Function

Private Function UpdateTimeFile(OBID As String) As String

  Dim buf As pString
  Dim Query As String
  Dim TransferPath As String
  Dim CacheSave As Boolean
     
  ' For checking in Cache
  Dim Entry As Object
  Dim Cache As Object
    
  Set ResultBuf = New Buffer
    
  Query = "upt " & OBID & " File"
  
  lPrint "Sending: " & Query
  
  If GetAnswer(Query, "qry.txt", buf) Then
    
    If buf.n > 0 Then
      UpdateTimeFile = buf.s
    End If
    
  Else

    lPrint "UpdateTimeFile fails"
  
  End If

End Function
' Davey - End Functions


