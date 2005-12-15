Attribute VB_Name = "modError"
Option Explicit

Public Const ERROR_RESPONSE = "Error: "
Public Const WARNING_RESPONSE = "Error: !"
Public Const MAGIC_KEY = "ol@tl98!"


#If OneBigApp Then
Function CheckReturnOK(fSpec As String) As Boolean
  
  Dim Handle As Long
  Dim FileLength As Long
  Dim FirstFewChars As String
  Dim ErrorMesg As String
  
  On Error GoTo CheckReturnOKErr
  
  If FileExists(fSpec) Then
    Handle = FreeFile
    Open fSpec For Binary Access Read As #Handle
    
    FileLength = LOF(Handle)
    If FileLength > 30 Then
      FirstFewChars = Space(30)
    Else
      FirstFewChars = Space(FileLength)
    End If
    Get #Handle, 1, FirstFewChars
    If InStr(FirstFewChars, ERROR_RESPONSE) <= 0 Then
      CheckReturnOK = True
    Else
      ErrorMesg = Space(FileLength)
      Get #Handle, 1, ErrorMesg
      frmError.labMsg = RemoveWarning(ErrorMesg)
      On Error Resume Next
      frmError.Show
      If Err = 0 Then
        SetWindowPos frmError.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW + SWP_NOMOVE + SWP_NOSIZE
      Else
        MsgBox frmError.labMsg, vbExclamation
        Unload frmError
      End If
      On Error GoTo 0
    End If
    Close #Handle
  Else
    frmError.labMsg = "File Does Not Exist:" + fSpec
    On Error Resume Next
    frmError.Show
    If Err = 0 Then
      SetWindowPos frmError.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW + SWP_NOMOVE + SWP_NOSIZE
    Else
      MsgBox frmError.labMsg, vbExclamation
      Unload frmError
    End If
    On Error GoTo 0
  End If
  
  Exit Function
  
CheckReturnOKErr:
  frmError.labMsg = Error
  On Error Resume Next
  frmError.Show
  If Err = 0 Then
    SetWindowPos frmError.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW + SWP_NOMOVE + SWP_NOSIZE
  Else
    MsgBox frmError.labMsg, vbExclamation
    Unload frmError
  End If
  
  Close #Handle
  
End Function
#End If

Public Function RemoveWarning(Msg As String) As String
  If Left(Msg, Len(WARNING_RESPONSE)) = WARNING_RESPONSE Then
    GetNext Msg, WARNING_RESPONSE
  End If
  RemoveWarning = Msg
End Function

