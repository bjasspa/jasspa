gobar1 <- function (go,data,replaceStr,letter,color, prop=FALSE,leg=FALSE) {
    attach(data)
    tab = table(Marker[level1 == go], 
       gsub("of biological process", "bp",
       gsub("  ", " ",
       gsub(" ", "\n",
       gsub(replaceStr, "", 
       sub("signal transducer activity", "signal transduction",
       sub("enzyme regulator", "enzyme",
       sub("enzyme regulator", "enzyme regul",
       sub("transcription regulator", "transcription regul",
       sub("structural molecule activity", "structural activity", 
         paste(level2[level1 == go], name2[level1 == go])))))))))))
   ylb = "Number of GO-annotated genes" 
   if (prop) { 
    tab = prop.table(tab,1)
     ylb = "GO-annotated genes"
   }
      bp = barplot2(tab, beside=T,legend.text=leg,cex.names=0.5,
          axes=T,ylab=ylb,
          cex.lab=0.5,cex.axis=0.5)
   # col=rainbow(12)
    # print (tab)
    # print (ftable(tab))
    #axis(3,at=bp,tick=F);
    mtext(letter,3,2,adj=0,cex=1)
    mtext(paste("       ",go,gonames$name[which(gonames$goID == go)]),3,2,adj=0,cex=0.6)
    detach(data)
  
   
}     
dg.getName <- function (go) {
    print 
}   
gobar2 <- function (go,data,replaceStr,letter,color,prop=FALSE,leg=FALSE) {
    attach(data)
    tab = table(Marker[level2 == go], 
    gsub("  ", " ",
   gsub(" ", "\n",
   gsub(replaceStr, "", paste(level3[level2 == go], name3[level2 == go])))))
    ylb = "Number of GO-annotated genes" 
   if (prop) { 
    tab = prop.table(tab,1)
     ylb = "GO-annotated genes"
   }
    bp = barplot2(tab, beside=T,legend.text=leg,cex.names=0.5,axes=T,ylab=ylb,
    cex.lab=0.5,cex.axis=0.5,col=color)
    mtext(letter,3,2,adj=0,cex=1)
     mtext(paste("      ",go,gonames$name[which(gonames$goID == go)]),3,2,adj=0,cex=0.6)
    #axis(1,at=bp,tick=F,labels=F)
    detach(data)
}     
gobar3 <- function (go,data,replaceStr,letter,color,prop=FALSE,leg=FALSE) {
    attach(data)
    tab = table(Marker[level3 == go], sub(" ", "\n",gsub(replaceStr, "", paste(level4[level3 == go], name4[level3 == go]))))
    ylb = "Number of GO-annotated genes" 
   if (prop) { 
    tab = prop.table(tab,1)
     ylb = "GO-annotated genes"
   }
    bp = barplot2(tab, beside=T,legend.text=leg,cex.names=0.6,axes=T,ylab=ylb,
    cex.lab=0.6,cex.axis=0.6)
    detach(data)
    mtext(paste(letter, go,gonames$name[which(gonames$goID == go)]),3,2,adj=0)
}     
library(gplots,verbose=F,warn.conflicts=F)
library(DBI)
library(RSQLite)
library(lattice)
m <- dbDriver("SQLite")
#jpeg("adjaye2004e1.jpeg",width = 960, height = 1280)
pdf("adjaye2004e1.pdf");
con <- dbConnect(m, dbname = "d:/data/adjaye2004-sebocytes.sqlite")
con2 <- dbConnect(m, dbname = "d:/data/adjaye2004.sqlite")
stm = paste("Select distinct Treatment as Marker,Gene,level1,level2,level3,name2,name3 ",
 "from stGene2GoNames where Treatment != 'tall' and name2 not like 'obsolete%' and name2 not like ",
"'%unknown' and name2 not like 'chaperone%' and level2 !=   'GO:0016209' and",
"level3 not in('GO:0046903', 'GO:0042592','GO:0050791','GO:0050817','GO:0043062','GO:0030235','GO:0043028','GO:0003823','GO:0008639','GO:0042562','GO:0004803','GO:0008144','GO:0019840','GO:0003969','GO:0008641','GO:0008367','GO:0008907','GO:0019842','GO:0003832','GO:0008987','GO:0008289','GO:0042165','GO:0005496')")
rs <- dbSendQuery(con, stm)
dataMarked <- fetch(rs, n = -1)  
stm2 = paste("Select distinct Treatment as Marker,Gene,level3,level4,name2,name3,name4 ",
 "from stGene2GoNames where Treatment != 'all' and name2 not like 'obsolete%' and name2 not like ",
"'%unknown' and name2 not like 'chaperone%' and level2 !=   'GO:0016209' and",
"level3 not in('GO:0046903', 'GO:0042592','GO:0050791','GO:0050817','GO:0043062','GO:0030235','GO:0043028','GO:0003823','GO:0008639','GO:0042562','GO:0004803','GO:0008144','GO:0019840','GO:0003969','GO:0008641','GO:0008367','GO:0008907','GO:0019842','GO:0003832','GO:0008987','GO:0008289','GO:0042165','GO:0005496')")
rs <- dbSendQuery(con, stm2)
dataMarked4 <- fetch(rs, n = -1)  
rs <- dbSendQuery(con, "select * from go2name")
gonames = fetch(rs, n = -1)  

par(mfrow=c(3,2))
par(mgp=c(2,1,0))
par(mar=c(2, 4, 4, 2) + 0.6)    
color = c("red","orange", "yellow","#999933")
color = c("red","orange", "yellow")
newstm = gsub("tall", "all", stm)
rs <- dbSendQuery(con, newstm)
dataMarked <- fetch(rs, n = -1)  
gobar1("GO:0003674",dataMarked, " activity", "A",color, F,leg=T);
color = c("red","orange", "yellow")
gobar2("GO:0004871",dataMarked, " activity", "B",color,F,leg=T);
gobar1("GO:0008150",dataMarked, " activity", " ",color, F);
color = c("orange","yellow")
gobar2("GO:0005198",dataMarked, " binding", " ", color,F);
color = c("red", "orange","yellow")
gobar1("GO:0005575",dataMarked, " activity", " ",color, F);
gobar2("GO:0030528",dataMarked, " activity", " ", color,F);
dev.off()
q()
gobar1("GO:0003674",dataMarked, " activity", "A",color, T,leg=T);
gobar1("GO:0008150",dataMarked, " activity", "B",color, T);
gobar1("GO:0005575",dataMarked, " activity", "C",color, T);

gobar2("GO:0004871",dataMarked, " activity", "A",color, T,leg=T);
color = c("red","orange", "yellow")
gobar2("GO:0005198",dataMarked, " activity", "B", color, T,leg=T);
color = c("red","orange", "yellow","#999933")
gobar2("GO:0030528",dataMarked, " activity", "C", color, T,leg=T);

color = c("red", "orange","yellow")
gobar1("GO:0003674",dataMarked, " activity", "A",color, T,leg=T);
color = c("red","orange", "yellow")
gobar1("GO:0008150",dataMarked, " activity", "B",color, T,leg=T);
color = c("red","orange", "yellow","#999933")
gobar1("GO:0005575",dataMarked, " activity", "C",color, T,leg=T);
gobar2("GO:0004871",dataMarked, " activity", "A",color,T);

gobar2("GO:0005198",dataMarked, " binding", "B", color,T,leg=T);
gobar2("GO:0030528",dataMarked, " activity", "C", color,T,leg=T);
# physiological process
gobar2("GO:0007582",dataMarked, " process", "A", color,T,leg=T);
gobar2("GO:0007582",dataMarked, " process", "B", color,F,leg=T);
gobar3("GO:0008152",dataMarked4, " process", "C", color,F,leg=T);
#gobar2("GO:0008369",dataMarked, " binding");
#gobar2("GO:0004871",dataMarked, " binding");
#gobar2("GO:0005215",dataMarked, " binding");
dev.off();
q()
