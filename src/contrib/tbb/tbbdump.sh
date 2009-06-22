#!/bin/bash
#

        DayTag=$(date +%a%d)
        MonthTag=$(date +%m)
        YearTag=$(date +%Y)

        export DireName="/home/eikelboom/data/hdf5data/$DayTag-$MonthTag-$YearTag/"
        mkdir -p "$DireName"

        export DumpLog="$DireName"dumplog.txt

        StartDate=$(date)
        echo " This is the TBB dump log file "  > "$DumpLog"
        echo ""                                 >> "$DumpLog"
        echo "$StartDate Start up"              >> "$DumpLog"

AlphaFun ()					# This function checks if input is alphabetic
{
        InputString="$1"

	NChars="$#"

        FlagOne=0
	FlagTwo=0

	if echo "$InputString" | grep -q [0-9]
	 then ((FlagOne=FlagOne+1))
	fi
	
	if echo "$InputString" | grep -iqv [a-z]
	 then ((FlagTwo=FlagTwo+1))
	fi

	((AlphaTest=FlagOne+FlagTwo))

	if [ "$AlphaTest" -ne 0 -o "$NChars" -eq 0 ]
	 then echo "Wrong Input for type Character" | tee "$DumpLog"
	 return 1
	fi
}


IntegFun ()					# This function checks if input is integer
{
	InputString="$1"

	NChars="$#"

	FlagOne=0
	FlagTwo=0

	  if echo "$InputString" | grep -iq [a-z]
	   then ((FlagOne=FlagOne+1))
	  fi
	  
	  if echo "$InputString" | grep -qv [0-9]
	    then ((FlagTwo=FlagTwo+1))
	  fi

        ((IntegTest=FlagOne+FlagTwo))	

	if [ "$IntegTest" -ne 0 -o "$NChars" -eq 0 ]
	 then echo "Wrong Input for type Integer" | tee "$DumpLog"
	 return 1
	fi
}


TestFun ()					# This function tries to restore failing TBB's
{
        TestTime=$(date)
        echo "$TestTime There was an error" | tee "$DumpLog"

        Station="$1"

        ssh "$Station"  tbbctl --stop   >> /dev/null
        ssh "$Station"  tbbctl --free   >> /dev/null
        ssh "$Station"  tbbctl --alloc  >> /dev/null

        sleep 2

	TestTime=$(date)

        ssh "$Station"  tbbctl --rec    >> /dev/null
        ssh "$Station"  tbbctl --stop   >> /dev/null

	TestTime=$(date)

	ssh "$Station"  tbbctl --free   >> /dev/null

        if [ "$?" -ne 0 ]
         then ssh "$Station"    tbbctl --clear
         echo "$TestTime Attempt to clear the tbb's"  | tee "$DumpLog"
         echo "Please wait 20 seconds..."
         sleep 20

	 TestTime=$(date)

	 ssh "$Station"  tbbctl --free   >> /dev/null

         if [ "$?" -ne 0 ]
          then ssh "$Station"   tbbctl --reset
          echo "$TestTime Clearing and subsequent freeing failed"    | tee "$DumpLog"
          echo "$TestTime Attempt to reset the tbb"                  | tee "$DumpLog"
          sleep 45
         fi

	TestTime=$(date)

        ssh "$Station"  tbbctl --free   >> /dev/null
        ssh "$Station"  tbbctl --alloc  >> /dev/null
        ssh "$Station"  tbbctl --rec    >> /dev/null
        ssh "$Station"  tbbctl --stop   >> /dev/null
        fi

	TestTime=$(date)

	ssh "$Station"  tbbctl --free   >> /dev/null

        if [ "$?" -ne 0 ]
         then echo "$TestTime The Transient Buffer Boards seem to be unstable" | tee \
         "$DumpLog"
         echo "$TestTime Exiting tbbdump" | tee "$DumpLog"
         sleep 2
         ExitFun "$Station"
        fi
}


SetFun ()					# This function sets up the RSP boards
{
        Station="$1"
        shift
        Frequency="$1"
        shift
        Counter=0

        until [ "$1" == "flag01" ]
         do
         RcuArray["$Counter"]="$1"
         shift
         ((Counter=Counter+1))
        done
        shift
        Counter=0

        until [ "$1" == "flag02" ]
         do
         ModeArray["$Counter"]="$1"
         shift
         ((Counter=Counter+1))
        done
        shift
        Counter=0

        TbbMode="$1"

        if ssh "$Station" rspctl --clock | grep -v "$Frequency"MHz
         then ssh "$Station" rspctl --clock="$Frequency"
        fi

        RcUnits="${RcuArray}"

       while [ "$Counter" -le "$RcUnits" ]
         do
         ssh "$Station" rspctl --rcumode="${ModeArray["$Counter"]}" --select="${RcuArray["$Counter"]}"
         ((Counter=Counter+1))
        done

        if ssh "$Station" rspctl --tbbmode | grep -v "$TbbMode"
         then ssh "$Station" rspctl --tbbmode="$TbbMode"
        fi
}


RecordFun ()					# This function instructs the TBB's to record data
{
        RecordTime=$(date)

        Station="$1"
        shift
        Counter=0

        until [ "$1" == "flag" ]
         do
         RcuArray["$Counter"]="$1"
         shift
        ((Counter=Counter+1))
        done
        shift

        RcuString=$(echo ${RcuArray[*]})
        RcuString=${RcuString// /,}

        ssh "$Station"  tbbctl --free

        if [ "$?" -ne 0 ]
         then echo "$RecordTime There was an error when the tbb's were cleared" | tee \
         "$DumpLog"
         TestFun "$Station"
        fi

        ssh "$Station"  tbbctl --alloc  --select="$RcuString"

        if [ "$?" -ne 0 ]
         then echo "$RecordTime There was an error when the RCU's were allocated" | tee \
         "$DumpLog"
         TestFun "$Station"
        fi

        ssh "$Station"  tbbctl --rec    --select="$RcuString"

        if [ "$?" -ne 0 ]
         then echo "$RecordTime There was an error when the recording took place" | tee |
         "$DumpLog"
         TestFun "$Station"
        fi

        sleep 0.5
        ssh "$Station"  tbbctl --stop   --select="$RcuString"

        if [ "$?" -ne 0 ]
         then TestFun "$Station"
	 else echo "$RecordTime Data was recorded" >> "$DumpLog"
        fi
}


DumpFun ()					# This function instructs to dump data to CEP
{
        DumpTime=$(date)

        Station="$1"
        shift
        Counter=0

        if [ "$Station" == "CS001C" ]
         then IPaddress="10.151.1.1"
         elif [ "$Station" == "CS010C" ]
          then IPaddress="10.151.19.1"
          elif [ "$Station" == "CS302C" ]
           then IPaddress="10.151.169.1"
        fi

        until [ "$1" == "flag" ]
         do
         RcuArray["$Counter"]="$1"
         shift
        ((Counter=Counter+1))
        done
        shift
        Counter=0

        RcuString=$(echo ${RcuArray[*]})
        RcuString=${RcuString// /,}

        NElements=${#RcuArray[*]}

        FlagPort=(0 0 0 0 0)

        BoardString=(0 0 0 0 0)

        while [ "$Counter" -le "$NElements" ]
         do

         RCUNumber=${RcuArray["$Counter"]}

         if [ "$RCUNumber" -ge 0 -a "$RCUNumber" -le 15 ]
          then FlagPort[0]=31664
          BoardString[0]="${BoardString[0]}","$RCUNumber"
          elif [ "$RCUNumber" -ge 16 -a "$RCUNumber" -le 31 ]
           then FlagPort[1]=31665
           BoardString[1]="${BoardString[1]}","$RCUNumber"
           elif [ "$RCUNumber" -ge 32 -a "$RCUNumber" -le 47 ]
            then FlagPort[2]=31666
            BoardString[2]="${BoardString[2]}","$RCUNumber"
            elif [ "$RCUNumber" -ge 48 -a "$RCUNumber" -le 63 ]
             then FlagPort[3]=31667
             BoardString[3]="${BoardString[3]}","$RCUNumber"
             elif [ "$RCUNumber" -ge 64 -a "$RCUNumber" -le 79 ]
              then FlagPort[4]=31668
              BoardString[4]="${BoardString[4]}","$RCUNumber"
              elif [ "$RCUNumber" -ge 80 -a "$RCUNumber" -le 95 ]
               then FlagPort[5]=31669
               BoardString[5]="${BoardString[5]}","$RCUNumber"
          fi

          ((Counter=Counter+1))

         done

        TbbMode="$1"
        shift

        Pages="$1"
        shift

        Latency="$1"
        shift
        Counter=0

        HourTag=$(date +%H)

        if echo "$HourTag" | grep -qw 0.
         then HourTag=${HourTag:1}
         ((HourTag=HourTag+2))
        fi

        MinuteTag=$(date +%M)
        SecondsTag=$(date +%S)

        while [ "$Counter" -le 5 ]
        do

         BoardString["$Counter"]=${BoardString["$Counter"]:2}
         Port=${FlagPort["$Counter"]}

         FileName="$Station-B$CounterT$HourTag:$MinuteTag:$SecondsTag".h5
         FullName="$DireName""$FileName"

         if [ "$Port" -ne 0 ]
          then nohup /bin/bash -e | ssh 10.181.0.3 /app64/usg/bin/tbb2h5 --ip "$IPaddress" \
          --port "$Port" --outfile "$FullName" --timeoutRead "$Latency" &
          ssh "$Station" tbbctl --readall="$Pages" --select="${BoardString[*]}"
         fi

         if ls -l "$FullName" | grep -qw [0,96,5384]
          then echo "$DumpTime The *.hdf5 file produced was useless"  | tee "$DumpLog"
          echo "$DumpTime Removing the last *.hdf5 file"              | tee "$DumpLog"
          rm -f "$FullName"
	  else echo "$DumpTime Data was dumped and converted at CEP" >> "$DumpLog"
         fi

        done
}


ExitFun()					# This function kills tbbdump and all childeren
{
	Station="$1"

        echo "Please wait while processes on external nodes are killed "
        echo "and local variables and functions are unset, if necessary"

        ssh "$Station" tbbctl --stop

        ProcId=$(ssh 10.181.0.3 ps -C tbb2h5 -u eikelboom -o pid=)
        ssh 10.181.0.3 kill -KILL "$ProcId"

        unset -f AdminFun IntegFun AlphaFun SetFun TestFun \
                 RecordFun DumpFun InputFun ExitFun

        ExitTime=$(date)
        echo "$ExitTime tbbdump stopped on user's request" | tee "$DumpLog"

        unset DireName
        unset DumpLog

        exit 0
}



#########################################################################################
#											#
#		This is the main body of the script; it traps user's input		#
#		and calls the functions defined above.					#
#											#
#########################################################################################


############## This part below scans and stores user's input ############################


InputFlag=("unset" "unset" "unset" "unset" "unset" "unset" "unset")	# Input analyzer


until [ "$#" -le 0 ]
 do

# Determine Station

	AlphaFun "$1"

	case "$1" in
	 s | -s | S | -S | --station)
	  shift

	  case "$1" in
	   cs001 | cs001c | CS001 | CS001C)
	    shift
	    Station="CS001C"
	    echo "$MainTime Station set to     : $Station" | tee "$DumpLog"
	    InputFlag[0]="set";;

	   cs010 | cs010c | CS010 | CS010C)
	    shift
	    Station="CS010C"
	    echo "$MainTime Station set to     : $Station" | tee "$DumpLog"
	    InputFlag[0]="set";;

	   cs016 | cs016c | CS016 | CS016C)
	    shift
	    Station="CS016C"
	    echo "$MainTime Station set to     : $Station" | tee "$DumpLog"
	    InputFlag[0]="set";;

	   cs302 | cs302c | CS302 | CS302C)
	    shift
	    Station="CS302C"
	    echo "$MainTime Station set to     : $Station" | tee "$DumpLog"
	    InputFlag[0]="set";;
	   *)
	    echo "Wrong Station Declaration; exiting tbbdump" | tee "$DumpLog"
	    sleep 2
	    exit 1;;
	  esac;;
	esac

# Determine Frequency

	AlphaFun "$1"

	case "$1" in
	 f | -f | F | -F | --frequency)
	  shift

	  IntegFun "$1"


	  if [ "$1" -eq 160 -o "$1" -eq 200 ]
	   then Frequency="$1"
	   shift
	   InputFlag[1]="set"
	   echo "$MainTime Frequency set to   : $Frequency" >> "$DumpLog"
	   else echo "$MainTime Wrong Declaration for Frequency; using current Station \
	   Frequency" | tee "$DumpLog"
	   InputFlag[1]="unset"
	   sleep 2
	  fi;;
	esac


# Determine Receiving Units

	AlphaFun "$1"

	case "$1" in
	 r | -r | R | -R | --rculist)
	  shift

	  IntegFun "$1"

          if [ "$?" -eq 1 ]
           then echo " Wrong Format used in defining RCU's; exiting tbbdump"
           sleep 2
           exit 1
          fi

          MainCounter=0

	  GroupCounter=0

	  AlphaFun "$1"

          until [ "$?" -eq 0 ]
           do

	   ColonFlag=0
	   CommaFlag=0

	   if echo "$1" | grep -q :
	    then ColonFlag=1
	   fi

	   if echo "$1" | grep -q ,
	    then CommaFlag=1
	   fi

	   ((OptFlag=ColonFlag*CommaFlag))

           RcuVars="$1"

	   IntegFun "$1"

           if echo "$1" | grep -q :
            then VarRange=${RcuVars/:/ }
            VarRange=($(echo $VarRange))

            VarElements=${#VarRange[@]}

	    RcuCluster["$GroupCounter"]="$VarElements"

            ((VarElements=VarElements-1))

            LowVar=$(echo ${VarRange[0]})
            HighVar=$(echo ${VarRange["$VarElements"]})

            ((VarLimit=HighVar-LowVar))

            if [ "$VarLimit" -lt 0 ]
             then ((VarLimit=-1*VarLimit))
             ((LowVar=LowVar-VarLimit))
             ((HighVar=HighVar+VarLimit))
            fi

            VarCounter=0

            while [ "$VarCounter" -le "$VarLimit" ]
             do

             RcuArray["$MainCounter"]="$LowVar"

             ((LowVar=LowVar+1))

             ((VarCounter=VarCounter+1))

             ((MainCounter=MainCounter+1))

            done

            elif echo "$1" | grep -q ,
             then VarRange=${RcuVars//,/ }
             VarRange=($(echo $VarRange))

             VarElements=${#VarRange[@]}

             ((VarLimit=VarElements-1))

	     RcuCluster["$GroupCounter"]="$VarElements"

             VarCounter=0

	     while [ "$VarCounter" -le "$VarLimit" ]
              do

              RcuArray["$MainCounter"]=$(echo ${VarRange["$VarCounter"]})

              ((VarCounter=VarCounter+1))

              ((MainCounter=MainCounter+1))

             done

            else RcuArray["$MainCounter"]="$1"

	    RcuCluster["$GroupCounter"]=1

	    ((MainCounter=MainCounter+1))
           fi

            if [ "$OptFlag" -eq 1 ]
             then echo "Wrongful combination of colon(s) and comma(s) in RCU declaration; \
             exiting tbbdump" | tee "$DumpLog"
             sleep 2
             exit 1
             else shift
            fi

            ((GroupCounter=GroupCounter+1))

	  AlphaFun "$1"

          done

          echo " RCU's set to       : ${RcuArray[*]}" | tee "$DumpLog"

          if [ "$MainCounter" -ge 1 ]
           then InputFlag[2]="set"
          fi;;

	esac


# Determine RCU Modes

	AlphaFun "$1"

	case "$1" in
	 m | -m | M | -M | --rcumode)
	  shift
	  MainCounter=0

	  IntegFun "$1"

	  until [ "$?" -eq 1 ]
	   do

	   ModeVars="$1"

	   if echo "$1" | grep -q -
	    then ModeRange=${ModeVars//-/ }
            ModeRange=($(echo $ModeRange))

	    Counter=0

	    NElements=${#ModeRange[@]}

	   until [ "$Counter" -eq "$NElements" ]
	    do

	    RcuMode["$MainCounter"]=$(echo ${ModeRange["$Counter"]})

	    ((Counter=Counter+1))
	
	    ((MainCounter=MainCounter+1))

	   done

	   else RcuMode["$MainCounter"]="$1"

            ((MainCounter=MainCounter+1))
          fi

	   shift

	  IntegFun "$1"

	  done

	  NRcus=$(echo ${#RcuArrays[@]})
	  NClusters=$(echo ${#RcuCluster[@]})

	  RcuCounter=0

	  until [ "$RcuCounter" -ge "$NRcus" ]	
           do

	  ClusterCounter=0 

	  NClusterRcus=$(echo ${RcuCluster["$RcuCounter"]})	
	
	  until [ "$ClusterCounter" -ge NClusterRcus ]
           do

           ModeArray["$RcuCounter"]=$(echo ${RcuMode["$ClusterCounter"]})

	   ((ClusterCounter=ClusterCounter+1))

	   ((RcuCounter=RcuCounter+1))

	  done

	  done	

	  echo "$MainTime RCU Modes set to   : ${ModeArray[*]}" >> "$DumpLog";;

	esac


# Determining TBB Mode

	AlphaFun "$1"

	case "$1" in
	 t | -t | T | -T | --transient)
	  TbbMode="transient"
	  shift
	  echo "$MainTime TBB Mode set to	: $TbbMode" >> "$DumpLog"
	  InputFlag[4]="set";;
	 u | -u | U | -U | --subband)
	  TbbMode="subband"
	  shift
	  echo "$MainTime TbbMode set to	: $TbbMode" >> "$DumpLog"
	  InputFlag[4]="set";;
	esac

	AlphaFun "$1"


# Determining the Number of Pages

	AlphaFun "$1"

	case "$1" in
	 p | -p | P | -P | --pages)
	  shift

	  IntegFun "$1"

	  if [ "$?" -eq 0 -a "$1" -gt 0 ]
	   then Pages="$1"
	   shift
	   echo "$MainTime Requested to dump   : $Pages pages" >> "$DumpLog"
	   InputFlag[4]="set"
	   else echo "$MainTime Pages set to default of 2000" | tee "$DumpLog"
	   InputFlag[4]="unset"
	   sleep 2
	  fi;;
	esac


# Determining the latency (i.e. sleep time) of tbb2h5

	AlphaFun "$1"

	case "$1" in
	 l | -l | L | -L | --latency)
	  shift

	  IntegFun "$1"

	  if [ "$?" -eq 0 ]
	   then Latency="$1"
	   shift
	   echo "$MainTime Latency set to      : $Latency s" >> "$DumpLog"
	   InputFlag[5]="set"
	   else echo "$MainTime Latency set to default of 1 s" | tee "$DumpLog"
	   InputFlag[5]="unset"
	   sleep 2
	  fi;;
	esac
 

# Get dumplog file or show helpfile

	case "$1" in
	 d | -d | D | -D | --dumplog)
	  cat "$DumpLog"
	  exit 0;;
	esac

	
        case "$1" in
         h | -h | H | -H | help | --help)
          cat ./tbbdumphelp.txt
          exit 0;;
        esac


done

########################################################################################


############# This section determines when TBBDUMP stops ###############################


        StopCommand=$(ExitFun "$Station")

        trap "$StopCommand" SIGHUP SIGINT SIGTERM SIGSTOP SIGURG

        MainTime=$(date)


########################################################################################


############## With the input determined, the repetitive dumps are started #############


MainTime=$(date)				# Redefining because some time has passed


############# Checking the RSP board settings, altering them if necessary ##############


echo "$MainTime Checking RSP board for current settings..." | tee "$DumpLog"

SetFun "$Station" "$Frequency" "${RcuArray[@]}" "flag01" "${ModeArray[@]}" "flag02" "$TbbMode"

echo "$MainTime Starting up the repetitive Dumps" | tee "$DumpLog"

echo ""
echo "Press Ctl-C to stop tbbdump"


############# Starting the repetitive dumping ###########################################


MainCounter=1

while :
 do

 MainTime=$(date)

 echo "$MainTime Start recording for Dump $MainCounter" >> "$DumpLog"

 RecordFun "$Station" "${RcuArray[@]}" "flag"

 echo "$MainTime Data is dumped for Dump $MainCounter" >> "$DumpLog"

 DumpFun "$Station" "${RcuArray[@]}" "flag" "$TbbMode" "$Pages" "$Latency"

 ((MainCounter=MainCounter+1))

done


############# END ############# END ############# END ############# END #################
#											#
#				End of TBBDUMP routine					#
#											#
############# END ############# END ############# END ############# END #################
