@startuml
' https://fontawesome.com/icons/database?s=solid
sprite database <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 448 512"><!--!Font Awesome Free v7.1.0 by @fontawesome - https://fontawesome.com License - https://fontawesome.com/license/free Copyright 2026 Fonticons, Inc.--><path d="M448 205.8c-14.8 9.8-31.8 17.7-49.5 24-47 16.8-108.7 26.2-174.5 26.2S96.4 246.5 49.5 229.8c-17.6-6.3-34.7-14.2-49.5-24L0 288c0 44.2 100.3 80 224 80s224-35.8 224-80l0-82.2zm0-77.8l0-48C448 35.8 347.7 0 224 0S0 35.8 0 80l0 48c0 44.2 100.3 80 224 80s224-35.8 224-80zM398.5 389.8C351.6 406.5 289.9 416 224 416S96.4 406.5 49.5 389.8c-17.6-6.3-34.7-14.2-49.5-24L0 432c0 44.2 100.3 80 224 80s224-35.8 224-80l0-66.2c-14.8 9.8-31.8 17.7-49.5 24z"/></svg>
<style>
rectangle {
  MinimumWidth 150
  FontColor black
  FontSize 18
  FontWeight bold
  HorizontalAlignment center
}
arrow {
  LineThickness 2
  FontSize 16
  FontWeight bold
  FontColor DarkBlue
}
</style>
rectangle "Data\nRequirements" as R1 #salmon 
rectangle "Data\nModelling" as R2 #skyblue
rectangle "Logical\nDesign" as R3 #skyblue
rectangle "Distribution\nDesign" as R4 #skyblue
rectangle "Physical\nDesign" as R5 #skyblue
rectangle "<$database{scale=0.04}>\nDatabase" as R6 #salmon

R1 ->  R2 
R2 --> R3 :  " <&key*1.2> ERD"   
R3 --> R4 : " <&spreadsheet*1.2> Tables"
R4 --> R5 : "  <&file*1.2> Files"
R5 ->  R6
@enduml
