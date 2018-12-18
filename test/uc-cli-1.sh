#!/bin/bash
trap "exit 1" TERM
TOP_PID=$$

function alert()
{
	        echo "$(tput setaf 1)Error!"
		 kill -s TERM $TOP_PID
}

if ./uc-cli showinfo |grep "message"; then
        alert
fi

if ./uc-cli showpeers|grep "message"; then
        alert
fi

if ./uc-cli addpeer 192.168.16.147:5682|grep "message"; then
	alert
fi
:<<BLOCK 
if ./uc-cli shutdown|grep "message"; then
       	alert
	fi
BLOCK
if ./uc-cli  createaccount ceshi1 123456|grep "message"; then
        alert
	fi

if ./uc-cli checkaccountinfo yujiali3 123456 island|grep "message" ; then
	        alert
	fi

if  ./uc-cli addaddress yujiali3 123456|grep "message"; then
                alert
	  fi

if  ./uc-cli validateaddress Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L|grep "code" -A1; then
       alert
 fi
if  ./uc-cli showaddresses yujiali3 123456|grep "message"; then 
	       alert 
	        fi
if  ./uc-cli exportkeyfile yujiali3 123456 island /test/ucd/bin|grep "message"; then                     
	       alert
	       fi
 if  ./uc-cli deleteaccount yujiali3 123456 island /test/ucd/bin|grep "message"; then
	 alert
 fi
 if  ./uc-cli changepass -p 12345 yujiali3 123456|grep "message"; then
	          alert 
		   fi
 if ./uc-cli importkeyfile yujiali3 123456  /test/ucd/bin/uc_keystore_yujiali3.json |grep "message"; then
	                   alert
			     fi

 :<<BLOCK if  ./uc-cli startmining snake02 123456 UcuW7wVu198Nuzok8eeMDUNEZQoGqQRRz5|grep "message"; then
    alert
    fi    

if ./uc-cli startmining snake03 123456 URaJN6xB2vmFGQcXDkLkrDjaSxu9qJN4Zr |grep "message"; then
	         alert
		     fi

if ./uc-cli stopmining snake02 123456 |grep "message"; then
	    alert
	        fi
BLOCK
if ./uc-cli showminers |grep "message"; then
	    alert
	        fi
if ./uc-cli showblockheight |grep "message"; then
	    alert
	        fi
if ./uc-cli showblock 324904 |grep "message"; then
    alert
   fi

if ./uc-cli showblockheader |grep "message"; then
           alert
       fi	
if ./uc-cli showheaderext 324904 |grep "message"; then
          alert
      fi	  
if ./uc-cli showmemorypool |grep "message"; then
      alert
    fi 
if ./uc-cli showtx 627e89bab348941810412ca966bd89959c6b2893e7d88385cc7204848c33a43a |grep "message"; then
        alert
    fi
if ./uc-cli showtxs yujiali3 123456 |grep -v "2003" |grep "code" -A1 ; then
         alert
      fi	  
var=$(./uc-cli createrawtx -r Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L:90000 -r UaeG98wu548vYTYTogvEevVPuC6ToUST3z:90000 -s UNfrtAxhJRi83PjTPjV3yNPKnjLYR22Bhx -t 0) 
echo $var 
if ./uc-cli createrawtx -r Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L:90000 -r UaeG98wu548vYTYTogvEevVPuC6ToUST3z:90000 -s UNfrtAxhJRi83PjTPjV3yNPKnjLYR22Bhx -t 0 |grep "message";then
	alert
fi

 if ./uc-cli decoderawtx $var |grep "message"; then
 	 alert
   fi
   var1=$(./uc-cli signrawtx yang 123456 $var)
     echo $var1    
if ./uc-cli signrawtx yang 123456 $var	|grep "message"; then
	    alert
	        fi 
./uc-cli sendrawtx $(echo $var1 | awk -F '"' '{print $8}')		




  a=$(./uc-cli createrawtx -r Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L:10000  -s Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L -t 1) 
 echo $a
if ./uc-cli createrawtx -r Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L:10000  -s Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L -t 1|grep "message";then
       alert
fi
if ./uc-cli decoderawtx $a |grep "message";then
	alert
fi
a1=$(./uc-cli signrawtx yujiali3 123456 $a)
echo $a1
./uc-cli sendrawtx $(echo $a1 | awk -F '"' '{print $8}')



./uc-cli createrawtx -n BLOCK -r Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L:9 -s UcuW7wVu198Nuzok8eeMDUNEZQoGqQRRz5 -t 3 

if ./uc-cli vote -r UcuW7wVu198Nuzok8eeMDUNEZQoGqQRRz5:9 yang 123456 UNfrtAxhJRi83PjTPjV3yNPKnjLYR22Bhx |grep "message"; then
    alert
    fi
   
./uc-cli createrawtx -n VOTE -r Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L:1 -s UcuW7wVu198Nuzok8eeMDUNEZQoGqQRRz5 -t 3


if ./uc-cli checkpublickey yujiali3 123456 Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L |grep "message"; then
    alert
    fi


if  ./uc-cli checkpublickey yujiali4 123456 UaeG98wu548vYTYTogvEevVPuC6ToUST3z |grep "message"; then
    alert
    fi


if ./uc-cli createmultisigaddress -m 2 -n 2 -s 03c8a6604e22c3f5174da08c79ab3a15f99817b529a6785dfc9816d351b2f17154 -k 03d8153cc69fc4f1e9a2c0904073faf253efd7a9580c925b365134bda74e942906 yujiali3 123456 |grep "message"; then
    alert
    fi

if ./uc-cli createmultisigaddress -m 2 -n 2 -s 03d8153cc69fc4f1e9a2c0904073faf253efd7a9580c925b365134bda74e942906 -k 03c8a6604e22c3f5174da08c79ab3a15f99817b529a6785dfc9816d351b2f17154 yujiali4 123456 |grep "message"; then
    alert
    fi

 ./uc-cli sendtokento snake02 123456 Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L BLOCK 9 
    

if ./uc-cli sendto yang 123456 3F81QMhhtBp4cK4WmDvYxRPVVfbEeZSV93  90000 |grep "message"; then
    alert
    fi
if ./uc-cli sendtokenfrom snake02 123456 UcuW7wVu198Nuzok8eeMDUNEZQoGqQRRz5 3F81QMhhtBp4cK4WmDvYxRPVVfbEeZSV93 BLOCK 9 |grep "message"; then
    alert
    fi



b=$(./uc-cli createmultisigtx -s BLOCK -t 3 yujiali3 123456 3F81QMhhtBp4cK4WmDvYxRPVVfbEeZSV93 Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L 1);
echo $b
if ./uc-cli createmultisigtx -s BLOCK -t 3 yujiali3 123456 3F81QMhhtBp4cK4WmDvYxRPVVfbEeZSV93 Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L 1 |grep "message";then
alert
fi
./uc-cli decoderawtx $b;
if ./uc-cli decoderawtx $b |grep "message";then
alert
fi
b1=$(./uc-cli signmultisigtx yujiali4 123456 $b);
echo $b1;
./uc-cli sendrawtx $(echo $b1 | awk -F '"' '{print $8}');

./uc-cli vote -r 3F81QMhhtBp4cK4WmDvYxRPVVfbEeZSV93:9 yang 123456 UNfrtAxhJRi83PjTPjV3yNPKnjLYR22Bhx;


c=$(./uc-cli createmultisigtx yujiali3 123456 3F81QMhhtBp4cK4WmDvYxRPVVfbEeZSV93 Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L 10000);
echo $c
if ./uc-cli createmultisigtx yujiali3 123456 3F81QMhhtBp4cK4WmDvYxRPVVfbEeZSV93 Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L 10000 |grep "message";then
alert
fi
./uc-cli decoderawtx $c;
if ./uc-cli decoderawtx $c |grep "message";then
alert
fi
c1=$(./uc-cli signmultisigtx yujiali4 123456 $c);
echo $c1;
./uc-cli sendrawtx $(echo $c1 | awk -F '"' '{print $8}');

if ./uc-cli showmultisigaddresses yujiali3 123456 |grep "message"; then
    alert
    fi
if ./uc-cli deletemultisigaddress yujiali3 123456 3F81QMhhtBp4cK4WmDvYxRPVVfbEeZSV93 |grep "message"; then
    alert
    fi

if ./uc-cli addaddress yujiali3 123456 |grep "message"; then
    alert
    fi
if ./uc-cli sendto yujiali3 123456 UVjsbgzEQZqqN3sM3MXrHwjvQ1MQggiXf9 10000 |grep "message"; then
    alert
    fi
if ./uc-cli sendfrom yang 123456 UNfrtAxhJRi83PjTPjV3yNPKnjLYR22Bhx Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L 80000 |grep "message"; then
    alert
    fi
if ./uc-cli sendtomulti -r Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L:10000 -r UaeG98wu548vYTYTogvEevVPuC6ToUST3z:100000 yang 123456 |grep "message"; then
    alert
    fi
if ./uc-cli deposit -a Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L yujiali3 123456 10000 |grep "message"; then
    alert
    fi
if ./uc-cli showbalances yujiali3 123456 |grep "message"; then
    alert
    fi
if ./uc-cli showbalance yujiali3 123456 |grep "message"; then
    alert
    fi
if ./uc-cli showaddressucn Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L |grep "message"; then
    alert
    fi
if ./uc-cli showtokens |grep "message"; then
    alert
    fi
if ./uc-cli showtoken |grep "message"; then
    alert
    fi
if ./uc-cli showaccounttoken yujiali3 123456 |grep "message"; then
    alert
    fi
if ./uc-cli showaddresstoken  Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L |grep "message"; then
    alert
    fi
if ./uc-cli sendtokenfrom snake03 123456 URaJN6xB2vmFGQcXDkLkrDjaSxu9qJN4Zr  Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L BLOCK 1  |grep "message"; then
    alert
    fi
 ./uc-cli sendtokenfrom snake02 123456 UcuW7wVu198Nuzok8eeMDUNEZQoGqQRRz5  Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L VOTE 1 
if ./uc-cli showaddresstoken  Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L |grep "message"; then
    alert
    fi
if ./uc-cli showtokens |grep "message"; then
    alert
    fi
 ./uc-cli destroy yujiali3 123456 BLOCK 1
./uc-cli destroy snake02 123456 VOTE 1 
if ./uc-cli sendtokenfrom yujiali3 123456 Ub9Utbs3hHMharWZBenvU8TjbygAXNtD5L 1111111111111111111114oLvT2 BLOCK 1 |grep "message"; then
    alert
    fi
/uc-cli sendtokenfrom snake02 123456 UcuW7wVu198Nuzok8eeMDUNEZQoGqQRRz5  1111111111111111111114oLvT2  VOTE 1 
    
if ./uc-cli showaddresstoken 1111111111111111111114oLvT2 |grep "message"; then
    alert
    fi

