c! to take .asrl file and write out associations in detail in .assf which is association file.
c! also to write another file to be used for plotting which is .pasf
c! modify to write name of source
c! modify to write .gasl which has all .gaul parameters only for associated sources. 
c! gasl will have the nearest second neighbour to every associated master source.

        subroutine write_associatelist1(nsrcm,totalm,dtotalm,peakm,
     /   dpeakm,ram,decm,bmajm,bminm,bpam,sstdm,savm,rstdm,ravm,nsrcs,
     /   totals,dtotals,peaks,dpeaks,ras,decs,bmajs,bmins,bpas,sstds,
     /   savs,rstds,ravs,tol,scrat,master,second,dsum,scratch,
     /   namesm,namess,rmsm,rmss,rcode,freq1,freq2)
        implicit none
        integer nsrcm,nsrcs,nchar,magic
        real*8 peakm(nsrcm),ram(nsrcm),decm(nsrcm),bmajm(nsrcm)
        real*8 bminm(nsrcm),bpam(nsrcm),rstdm(nsrcm),ravm(nsrcm)
        real*8 totalm(nsrcm),dpeakm(nsrcm),dtotalm(nsrcm)
        real*8 rmsm(nsrcm),rmss(nsrcs)
        real*8 peaks(nsrcs),ras(nsrcs),decs(nsrcs),bmajs(nsrcs)
        real*8 bmins(nsrcs),bpas(nsrcs),tol,dist,distold
        real*8 totals(nsrcs),dpeaks(nsrcs),dtotals(nsrcs)
        real*8 rstds(nsrcs),ravs(nsrcs),freq1,freq2
        real*8 sstds(nsrcs),savs(nsrcs),sstdm(nsrcm),savm(nsrcm)
        character scrat*500,extn*20,fn*500,fn1*500,str1*1,str8*8
        character fmt1*2,fmt2*2,fmti*100,fn2*500,dir*500,fmtj*100
        character master*(*),second*(*),dumc*500,scratch*500
        character namesm(nsrcm)*30,namess(nsrcs)*30,fn3*500,fmtd*100
        character str*10,fmte*100,rcode*2
        integer isrcm,isrcs,i,dumi,n_src_m2s,coden,nmul,j,mularr(nsrcm)
        integer nmult,dsum(5),idm,mlen,slen
       
        extn='.asrl'
        fn=scratch(1:nchar(scratch))//scrat(1:nchar(scrat))//
     /     extn(1:nchar(extn))
        extn='.assf'
        fn1=scratch(1:nchar(scratch))//scrat(1:nchar(scrat))//
     /     extn(1:nchar(extn))
        extn='.pasf'
        fn2=scratch(1:nchar(scratch))//scrat(1:nchar(scrat))//
     /     extn(1:nchar(extn))
        extn='.gasl'
        fn3=scratch(1:nchar(scratch))//scrat(1:nchar(scrat))//
     /     extn(1:nchar(extn))
        if (rcode(1:1).ne.'q') then
         write (*,*) 
         write (*,*) '  Writing out '//fn1(1:nchar(fn1))
         write (*,*) '  Writing out '//fn2(1:nchar(fn2))
         write (*,*) '  Writing out '//fn3(1:nchar(fn3))
        end if
        open(unit=21,file=fn,status='old')
        open(unit=22,file=fn1,status='unknown')
        open(unit=23,file=fn2,status='unknown')
        open(unit=24,file=fn3,status='unknown')
        write (23,*) freq1,freq2
        call sub_write_assoc_head(master,second,nsrcm,nsrcs,tol,22,dsum)
        call sub_wr_asrl_getmaxlen_names(namesm,namess,nsrcm,nsrcs,
     /       mlen,slen)
        call int2str(mlen,str)
        if (mlen.eq.0) str='1'
        fmte="(i7,1x,a"//str(1:nchar(str))//",1x,i6,1x,a"
        call int2str(slen,str)
        if (slen.eq.0) str='1'
        fmte=fmte(1:nchar(fmte))//str(1:nchar(str))//
     /       ",1x,19(1Pe15.8,1x))"
        write (24,*) 'Master_source_list ',master(1:nchar(master))
        write (24,*) 'Second_source_list ',second(1:nchar(second))
        write (24,*) 'Number_of_associations ',dsum(1)
        write (24,*) 'id1 name1 id2 name2 tot1 etot1 tot2 etot2 peak1'//
     /   ' epeak1 peak2 epeak2 ra1 dec1 ra2 dec2 bmaj1 bmin1 bpa1 '//
     /   'bmaj2 bmin2 bpa2 dist'
        read (21,*) magic                               ! signifies EOF
444     read (21,*) dumi
        if (dumi.eq.magic) then
         goto 333
        else
         backspace(21)
        end if
        read (21,*) isrcm,n_src_m2s
        call get_fmt_int(isrcm,fmt1)
        call get_fmt_int(n_src_m2s,fmt2)
        if (isrcm.eq.0) then 
         call write_srclist_singlekothi(n_src_m2s,22,namess(n_src_m2s))
        else
         fmti="(a14,"//fmt1//",a8,"//fmt2//",a13)"
         fmtj="(a14,"//fmt1//",a8,a30)"
         write (22,fmti) 'Master source ',isrcm,' ;  Has ',
     /                n_src_m2s,' associations'
         if (n_src_m2s.eq.0) write (22,fmtj) 'Master source ',isrcm,
     /       ' name : ',namesm(isrcm)
         distold=9.d99
         idm=99999
         do i=1,n_src_m2s
          read (21,*) isrcs,str1 
          if (str1.eq.'s') str8='singly  '
          if (str1.eq.'m') str8='multiply'
          call get_fmt_int(isrcs,fmt1)
          fmti="(a25,"//fmt1//",a11,a8,a11)"
          write (22,fmti) 'Associated sub source is ',isrcs,
     /       ', which is ',str8,' associated'
          call justdist(ram(isrcm),ras(isrcs),decm(isrcm),decs(isrcs),
     /         dist)
          if (dist.lt.distold) then
           idm=isrcs
           distold=dist
          end if
         end do
         do i=1,n_src_m2s
          backspace(21)
         end do
         coden=2
         do i=1,n_src_m2s
          read (21,*) isrcs,str1 
          if (i.ne.1) coden=1
          call sub_write_assoclist1(totalm(isrcm),dtotalm(isrcm),
     /     peakm(isrcm),dpeakm(isrcm),ram(isrcm),decm(isrcm),
     /     bmajm(isrcm),bminm(isrcm),bpam(isrcm),totals(isrcs),
     /     dtotals(isrcs),peaks(isrcs),dpeaks(isrcs),
     /     ras(isrcs),decs(isrcs),bmajs(isrcs),bmins(isrcs),
     /     bpas(isrcs),sstds(isrcs),rstds(isrcs),22,coden,isrcm,
     /     isrcs,'Master ','Second ',namesm(isrcm),namess(isrcs))
          if (str1.eq.'s') then
           write (23,777) isrcm,totalm(isrcm),dtotalm(isrcm),
     /      peakm(isrcm),dpeakm(isrcm),ram(isrcm),decm(isrcm),
     /      bmajm(isrcm),bminm(isrcm),bpam(isrcm),sstds(idm),rstds(idm),
     /      rmsm(isrcm)
           write (23,777) idm,totals(idm),dtotals(idm),peaks(idm),
     /      dpeaks(idm),ras(idm),decs(idm),bmajs(idm),bmins(idm),
     /      bpas(idm),distold,rmss(idm)
           if (namesm(isrcm).eq."") namesm(isrcm)=' '
           if (namess(idm).eq."") namess(idm)=' '
           write (24,fmte) isrcm,namesm(isrcm),idm,namess(idm),
     /      totalm(isrcm),dtotalm(isrcm),totals(idm),dtotals(idm),
     /      peakm(isrcm),dpeakm(isrcm),peaks(idm),dpeaks(idm),
     /      ram(isrcm),decm(isrcm),ras(idm),decs(idm),
     /      bmajm(isrcm),bminm(isrcm),bpam(isrcm),
     /      bmajs(idm),bmins(idm),bpas(idm),distold
          end if
         end do
        end if
        write (22,*)
        goto 444
333     continue
        close(21)
c!                                              now for slutkothis
        call system('rm -f a b')
        call system('grep m '//fn(1:nchar(fn))//' > a')
        call system('sort -u -g a > b')
        dumc='b'
        dir="./"
        call getline(dumc,dir,nmult)
        open(unit=21,file='b',status='old')
        do i=1,nmult
         read (21,*) isrcs,str1,nmul
         call sub_write_assoc_getmulsrcid(21,nmul,mularr,nsrcm)
         call get_fmt_int(isrcs,fmt1)
         fmti="(a13,1x,"//fmt1//",1x,a22)"
         write (22,fmti) 'Second source',isrcs,'is multiply associated'
         coden=2
         distold=9.d99
         idm=99999
         do j=1,nmul
          if (j.ne.1) coden=1
          call sub_write_assoclist1(totals(isrcs),dtotals(isrcs),
     /     peaks(isrcs),dpeaks(isrcs),ras(isrcs),decs(isrcs),
     /     bmajs(isrcs),bmins(isrcs),bpas(isrcs),totalm(mularr(j)),
     /     dtotalm(mularr(j)),peakm(mularr(j)),dpeakm(mularr(j)),
     /     ram(mularr(j)),decm(mularr(j)),bmajm(mularr(j)),
     /     bminm(mularr(j)),bpam(mularr(j)),sstds(isrcs),rstds(isrcs),
     /     22,coden,isrcs,mularr(j),'Second ','Master ',
     /     namesm(mularr(j)),namess(isrcs))
          call justdist(ram(mularr(j)),ras(isrcs),decm(mularr(j)),
     /         decs(isrcs),dist)
          if (dist.lt.distold) then
           idm=mularr(j)
           distold=dist
          end if
         end do
         write (23,777) idm,totalm(idm),dtotalm(idm),peakm(idm),
     /    dpeakm(idm),ram(idm),decm(idm),bmajm(idm),bminm(idm),
     /    bpam(idm),sstds(isrcs),rstds(isrcs),rmsm(idm)
         write (23,777) isrcs,totals(isrcs),dtotals(isrcs),peaks(isrcs),
     /    dpeaks(isrcs),ras(isrcs),decs(isrcs),bmajs(isrcs),
     /    bmins(isrcs),bpas(isrcs),distold,rmss(isrcs)
         write (22,*)
        end do
        close(21)
        close(22)
        close(23)
        close(24)
777     format(i7,1x,15(1Pe15.8,1x))
778     format(i6,1x,a30,1x,i6,1x,a30,1x,19(1Pe15.8,1x))
        
        return
        end
c!
c! ----------------------------   SUBROUTINES   --------------------------------
c!
        subroutine sub_write_assoclist1(tot1,dtot1,peak1,dpeak1,ra1,
     /   dec1,bmaj1,bmin1,bpa1,tot2,dtot2,peak2,dpeak2,ra2,dec2,bmaj2,
     /   bmin2,bpa2,sstd,rstd,nn,coden,idm,ids,f1,f2,name1,name2)
        implicit none
        real*8 tot1,dtot1,dpeak1,peak1,ra1,dec1,bmaj1,bmin1,bpa1,ss1,sa1
        real*8 tot2,dtot2,dpeak2,peak2,ra2,dec2,bmaj2,bmin2,bpa2,dist
        real*8 ss2,sa2,rstd,sstd
        integer hh1,hh2,dd1,dd2,mm1,mm2,ma1,ma2,nn,coden,idm,ids,nchar
        character s1,s2,fmt1*2,fmta*500,f1*7,f2*7,rstd1*18,rstd2*18
        character fmta1*500,fmta2*500,sstd1*18,sstd2*18,name1*30
        character fmtb1*100,name2*30

        call justdist(ra1,ra2,dec1,dec2,dist)
        call convertra(ra1,hh1,mm1,ss1)
        call convertra(ra2,hh2,mm2,ss2)
        call convertdec(dec1,s1,dd1,ma1,sa1)
        call convertdec(dec2,s2,dd2,ma2,sa2)
        call get_fmt_int(max(idm,ids),fmt1)
        fmta="(a7,"//fmt1//",a3,0Pf10.4,a8,i2,1x,i2,1x,0Pf5.2,1x,"//
     /    "a1,i2,1x,i2,1x,0Pf5.2,1x,a2,0Pf10.3,a3,0Pf10.3,a3,0Pf7.3"
        fmtb1="(a7,"//fmt1//",a8,a30)"
        if (f1.eq.'Second ') then
         fmta1=fmta(1:nchar(fmta))//",a18,a18)"
         fmta2=fmta(1:nchar(fmta))//",a1,a9,a9,0Pf10.3,a1)"
         write (sstd1,'(a8,0Pf10.4)') '; rms : ',sstd
         write (rstd1,'(a8,0Pf10.4)') '; rms : ',rstd
         sstd2=' '
         rstd2=' '
        end if
        if (f1.eq.'Master ') then
         fmta1=fmta(1:nchar(fmta))//",a1)"
         fmta2=fmta(1:nchar(fmta))//",a18,a18,a9,0Pf10.3,a1)"
         write (sstd2,'(a8,0Pf10.4)') '; rms : ',sstd
         write (rstd2,'(a8,0Pf10.4)') '; rms : ',rstd
         sstd1=' '
         rstd1=' '
        end if
        if (coden.eq.2) then
         write (nn,fmta1) f1,idm,' : ',peak1,' Jy; At ',hh1,mm1,
     /        ss1,s1,dd1,ma1,sa1,'; ',bmaj1,' X ',bmin1,' X ',bpa1,
     /        sstd1,rstd1
         write (nn,fmtb1) f1,idm,' name : ',name1
        end if
        write (nn,fmta2) f2,ids,' : ',peak2,' Jy; At ',hh2,mm2,
     /        ss2,s2,dd2,ma2,sa2,'; ',bmaj2,' X ',bmin2,' X ',bpa2
     /        ,sstd2,rstd2,'; Dist = ',dist,'"'
         write (nn,fmtb1) f2,ids,' name : ',name2
        
        return
        end
c!
c!      ----------
c!
        subroutine write_srclist_singlekothi(ids,nn,names)
        implicit none
        integer ids,nn,nchar
        character fmt1*2,fmta*100,names*30,fmtb*100

        call get_fmt_int(ids,fmt1)
        fmta="(a14,"//fmt1//",a23,a11)"
        fmtb="(a14,"//fmt1//",a8,a30)"
        write (nn,fmta) 'Second source ',ids,' has no association in '
     /        ,'master list'
        write (nn,fmtb) 'Second source ',ids,' name : ',
     /        names(1:nchar(names))
        
        return
        end
c!
c!      ----------
c!
        subroutine sub_write_assoc_head(master,second,nm,ns,tol,nn,dsum)
        implicit none
        character master*(*),second*(*)
        integer nm,ns,nn,nchar,dsum(5)
        real*8 tol

        write (nn,*)
        write (nn,'(10x,a)') 'Association of Sourcelist file'
        write (nn,'(10x,a)') '=============================='
        write (nn,*)
        write (nn,*) 'Master source list : ',master(1:nchar(master)),
     /        ' (',nm,' srcs)'
        write (nn,*) 'Second source list : ',second(1:nchar(second)),
     /        ' (',ns,' srcs)'
        write (nn,*) 'Tolerance for association : ',tol,' arcsec'
        write (nn,*) 
        write (nn,*) 'Found ',dsum(1),' associations with ',
     /       master(1:nchar(master))
        write (nn,*)  'Sources in ',master(1:nchar(master)),
     /       ' with no associations is ',dsum(2)
        write (nn,*)  'Sources in ',second(1:nchar(second)),
     /       ' with no associations is ',dsum(3)
        write (nn,*)  'Sources in ',master(1:nchar(master)),
     /       ' with multiple associations is ',dsum(4)
        write (nn,*)  'Sources in ',second(1:nchar(second)),
     /       ' with multiple associations is ',dsum(5)
        write (nn,*) 

        return
        end
c!
c!      ----------
c!
        subroutine sub_write_assoc_getmulsrcid(nn,nmul,mularr,x)
        implicit none
        integer nn,x,mularr(x),nmul,vec(nmul),dumi,i
        character dumc

        backspace(nn)
        read (nn,*) dumi,dumc,dumi,vec
        do i=1,nmul
         mularr(i)=vec(i)
        end do

        return
        end
c!
c!      ----------
c!
        subroutine sub_wr_asrl_getmaxlen_names(namesm,namess,nsrcm,
     /             nsrcs,mlen,slen)
        implicit none
        integer nsrcm,nsrcs,mlen,slen,i,nchar
        character namesm(nsrcm)*30,namess(nsrcs)*30

        mlen=nchar(namesm(1))
        do i=1,nsrcm
         if (nchar(namesm(i)).gt.mlen) mlen=nchar(namesm(i))
        end do

        slen=nchar(namess(1))
        do i=1,nsrcs
         if (nchar(namess(i)).gt.slen) slen=nchar(namess(i))
        end do

        return
        end

