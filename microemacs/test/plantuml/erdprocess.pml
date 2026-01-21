@startuml
!include sprites.pml
<style>
rectangle {
  MinimumWidth 150
  FontColor black
  FontSize 18
  FontWeight bold
  HorizontalAlignment center
}
.salmon {
    BackgroundColor salmon
}   
arrow {
  LineThickness 2
  FontSize 16
  FontWeight bold
  FontColor DarkBlue
}
</style>
rectangle "Data\nRequirements" as R1 <<salmon>>
rectangle "Data\nModelling" as R2 #skyblue
rectangle "Logical\nDesign" as R3 #skyblue
rectangle "Distribution\nDesign" as R4 #skyblue
rectangle "Physical\nDesign" as R5 #skyblue
rectangle "<$database{scale=0.04}>\nDatabase" as R6 #cornsilk

R1 ->  R2 : " "
R2 --> R3 : " <&key*1.2> ERD"   
R3 --> R4 : " <&spreadsheet*1.2> Tables"
R4 --> R5 : "  <&file*1.2> Files"
R5 ->  R6
@enduml
