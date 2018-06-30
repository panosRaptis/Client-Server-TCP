#!/bin/bash

root_dir=$1; text_file=$2; w=$3; p=$4

argc=$#
if [ $argc -ne 4 ]; then # check argc
    echo "# Error: not enought arguments ..."; exit 1
fi

if [[ "$root_dir" =~ '/'$ ]]; then # if exists "/" (last character) remove it 
    root_dir=${root_dir:0:-1}
fi

if [ ! -d "$root_dir" ]; then
    echo "# Warning: directory not found, creating ...";
	mkdir $root_dir # create root_dir if not already exists
elif [ "$(ls -A $root_dir)" ]; then
    echo "# Warning: directory if full, purging..."
    for i in `ls -A $root_dir`; do # or if allready exists, purge it
        temp=$(printf '%s/%s' "$root_dir" "$i")
        rm -rf $temp
    done
fi

if [ ! -f "$text_file" ]; then
    echo "# Error: \"$text_file\" not found ..."; exit 1
fi

lines=$(cat "$text_file" | wc -l) # check if text_file >= 10.000 lines
if [ "$lines" -lt "10000" ]; then
    echo "# Error: \"$text_file\" has less than 10.000 lines ..."; exit 1
fi

if ! [[ "$w" =~ ^[0-9]+$ ]]; then # check if w is possitive number
    echo "# Error: \"$w\" is invallid number ..."; exit 1
fi

if ! [[ "$p" =~ ^[0-9]+$ ]]; then # check if p is possitive number
    echo "# Error: \"$p\" is invallid number ..."; exit 1
fi

if [ "$w" == "1" ] || [ "$p" == "1" ] || [ "$w" == "2" ] || [ "$p" == "2" ]; then # Error: marginal cases (by convention)
    echo "# Error !!"; exit 1
fi

declare -a startSiteArray=()
declare -a randomArray=()
declare -a pageArray
declare -a incomingLinkArray=()
pos=0
posArray=0
for ((i=0; i<$w; i++)); do # for each webSite folder
    site=$(printf 'site%d' "$i")
    temp=$(printf '%s/%s' "$root_dir" "$site")
    if [ ! -d $temp ]; then
            mkdir -p $temp; # create folder (dir)
    fi
    startSiteArray[posArray]=$pos # pos in randomArray  of 1st html file of i-th webSite
    for((q=0; q<$p; q++)); do # for each html file in each webSite Folder
        while : ; do 
            flag=0
            pageN=$RANDOM
            dirPage=$(printf '%s/page%d_%d.html' "$site" "$i" "$pageN")
            for ((j=$pos; j<${#randomArray[@]}; j++)); do
                if [ "$dirPage" == "${randomArray[$j]}" ]; then
                    flag=1
                    break
                fi
            done
            if [ "$flag" -eq "0" ]; then
                randomArray[${#randomArray[@]}]="$dirPage" # add unique dirPage in randomArray
                incomingLinkArray[${#incomingLinkArray[@]}]=$flag
                break
            fi
        done
    done 
    posArray=$(($posArray + 1))
    pos=$(($pos + $p)) # update pos
done  

for ((l=0; l<$w; l++)); do # for each webSite folder
    pos="${startSiteArray[$l]}"
    max=$(($pos + $p - 1))
    echo "# Create web site $l ..."
    for((v=0; v<$p; v++)); do # for each html file (page) in webSite folder
        pageArray=()
        a=`expr $l \* $p`
        a=$(($a + $v))
        k=$(seq 2 $(($lines - 1)) | shuf -n 1)
        m=$(seq 1001 1999 | shuf -n 1)
        f=$((`expr $p / 2` + 1))
        q=$((`expr $w / 2` + 1))
        for((z=0; z<$f; z++)); do
            while : ; do 
                flag=0
                posRandom=$(seq $pos $max | shuf -n 1) 
                tempPage="${randomArray[$posRandom]}"
                for ((j=0; j<${#pageArray[@]}; j++)); do
                    if [ "$tempPage" == "${pageArray[$j]}" ] || [ "$tempPage" == "${randomArray[$a]}" ]; then
                        flag=1
                        break
                    fi
                done
                if [ "$flag" -eq "0" ]; then
                    pageArray[${#pageArray[@]}]="$tempPage" # add in pageArray unique tempPage-s (Internal Links)
                    incomingLinkArray[posRandom]=$((1)) # we have incoming link here !!
                    break
                fi 
            done 
        done

        for((z=0; z<$q; z++)); do 
            while : ; do 
                flag=0
                strlenRA=${#randomArray[@]}
                strlenRA=$(($strlenRA - 1))
                numRandom=$(seq 0 $strlenRA | shuf -n 1) 
                if [ "$numRandom" -lt "$pos" ] || [ "$numRandom" -ge $(($pos + $p)) ]; then
                    tempWeb="${randomArray[$numRandom]}"
                    for ((j=$f; j<${#pageArray[@]}; j++)); do
                        if [ "$tempWeb" == "${pageArray[$j]}" ]; then
                            flag=1
                            break
                        fi
                    done
                    if [ "$flag" -eq "0" ]; then
                        pageArray[${#pageArray[@]}]="$tempWeb" # add in pageArray unique tempPage-s (External Links)
                        incomingLinkArray[numRandom]=$((1)) # we have incoming link here !!
                        break
                    fi 
                fi                    
            done
        done

        fullPathPage=$(printf '%s/%s' "$root_dir" "${randomArray[$a]}") # writing html file ...
        numLinesRead=$((`expr $m / $(($f + $q))`))        
        echo "#  Create page $fullPathPage with $m lines starting at line $k ..."
        printf "<!DOCTYPE html>\n<html>\n\t<body>\n" > "$fullPathPage" 
        pageArray=( $(shuf -e "${pageArray[@]}") )
        for((j=0; j<$(($f + $q)); j++)); do
            if [ $j -eq $(($f + $q - 1)) ]; then
                b=`expr $m % $(($f + $q))`
                numLinesRead=$(($numLinesRead + $b))
            fi
            numLinesFromTop=$(($k + $numLinesRead - 1))
            temp=$(printf 'link%d_text' "$j")
            cat $text_file | head -n $numLinesFromTop | tail -n $numLinesRead | sed -e 's/^/\t/' | head -c -1 >> "$fullPathPage"
            link=$(printf '/%s' "${pageArray[$j]}")
            echo "#   Adding link to $root_dir$link"
            printf " <a href=\"$link\">$temp</a>\n" >> "$fullPathPage"    
            k=$(($k + $numLinesRead))
        done
        printf "\t</body>\n</html>" >> "$fullPathPage"
    done
done

flag=0
for ((j=0; j<${#incomingLinkArray[@]}; j++)); do # print appropriate message for incomming links or not
    if [ "${incomingLinkArray[$j]}" -eq "0" ]; then
        flag=1
        echo "# At least one page has no one incoming link"
        break
    fi
done
if [ "$flag" -eq "0" ]; then
    echo "# All pages have at least one incoming link"
else
    echo "# At least one page has none a link"
fi
echo "# Done." # end of Bash :)
