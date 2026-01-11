#include <FileConstants.au3>
#include <MsgBoxConstants.au3>

Example()

Func Example()
	; Create a constant variable in Local scope of the filepath that will be read/written to.
	Local Const $sFilePath = @TempDir & "\FileCopy.au3"

	; Create a temporary file to copy.
	If Not FileCreate($sFilePath, "This is an example of using FileCopy.") Then Return MsgBox($MB_SYSTEMMODAL, "", "An error occurred whilst writing the temporary file.")

	; Copy Au3 files in the temporary directory to a new folder/directory called Au3Files.
	FileCopy(@TempDir & "\*.au3", @TempDir & "\Au3Files\", $FC_OVERWRITE + $FC_CREATEPATH)

	; Display the temporary directory.
	ShellExecute(@TempDir)
EndFunc   ;==>Example

; Create a file.
Func FileCreate($sFilePath, $sString)
	Local $fReturn = True ; Create a variable to store a boolean value.
	If FileExists($sFilePath) = 0 Then $fReturn = FileWrite($sFilePath, $sString) = 1 ; If FileWrite returned 1 this will be True otherwise False.
	Return $fReturn ; Return the boolean value of either True of False, depending on the return value of FileWrite.
EndFunc   ;==>FileCreate
