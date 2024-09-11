#!/usr/bin/env bash

function lang2rb () {
    ## creating receipe files from one template
    declare -A dict
    dict["dede"]="German"
    dict["elgr"]="Greek"    
    dict["engb"]="British"
    dict["enus"]="American"
    dict["eses"]="Spanish"
    dict["fifi"]="Finish"
    dict["frfr"]="French"
    dict["itit"]="Italian"   
    dict["plpl"]="Polish"
    dict["ptpt"]="Portuguese"    
    dict["ruru"]="Russian"        
    
    if [ -z $3 ]; then
        echo "Usage: lang2rb sha256file template-file"
        return
    else
        shift 1
        version=$(echo $1 | sed -E 's/.+MicroEmacs_([0-9]+).+/\1/') #'
        for key in "${!dict[@]}"; do
            Lang="${key^}"
            Language=${dict[$key]}
            sha=$(grep spelling_${key} $1 | sed -E 's/ .+//')
            outfile=$(echo $2 | sed -E "s/-template.tpl/-${key}.rb/")
            #echo -e "lang=${key} Lang=${Lang} Language=${Language}\tOutfile=$outfile\t${sha}" 
            cat microemacs-spelling-template.tpl | sed -E "s/__Lang__/${Lang}/g ; s/__Language__/${Language}/g; s/__lang__/${key}/g; s/__version__/${version}/g; s/__sha256__/${sha}/g;" > ${outfile}
            echo "${outfile} created"
        done
    fi
}

function mec2rb {
    ## creating one recipe file with multiple Zip file
    ## for mec, mew, mecs, mews and openssl release files
    if [ -z $3 ]; then 
        printf "mec2rb shafile templatefile\n"
    else
        shift 1
        version=$(echo $1 | sed -E 's/.+MicroEmacs_([0-9]+).+/\1/') #'
        outfile=$(echo $2 | sed -E 's/tpl$/rb/')
        declare -A dict
        dict["linux"]=""
        dict["macos_apple"]=""
        dict["macos_intel"]=""
        dict["windows"]=""     
        zip="binaries.zip"
        if [ "$(echo $2 | grep openssl)" != "" ]; then
            zip="openssl.zip"
        elif [ "$(echo $2 | grep mecs)" != "" ]; then
            zip="mecs.zip"
        elif [ "$(echo $2 | grep mews)" != "" ]; then
            zip="mews.zip"
        fi
        ## collecting hashes
        for key in "${!dict[@]}"; do
           sha=$(grep  ${key}_${zip} $1 | sed -E 's/ .+//g')
           dict["$key"]="$sha"
           #printf "outfile=%s %s\t = %s\n" $outfile $key $sha
        done
        cat $2 | sed "s/__version__/${version}/g; 
            s/__sha_macos_apple__/${dict['macos_apple']}/g;
            s/__sha_macos_intel__/${dict['macos_intel']}/g;
            s/__sha_linux__/${dict['linux']}/g;
            s/__sha_windows__/${dict['windows']}/g" > $outfile
        echo "$outfile created"
    fi
}
function emf2rb {
    ## process macro files like macros.zip help_ehf.zip
    if [ -z $3 ]; then 
        printf "emf2rb shafile templatefile\n"
    else
        shift 1
        version=$(echo $1 | sed -E 's/.+MicroEmacs_([0-9]+).+/\1/') #'
        outfile=$(echo $2 | sed -E 's/tpl$/rb/')
        macro="macros"
        if [ "$(echo $2 | grep help)" != "" ]; then
            macro="help_ehf"
        fi
        sha=$(grep  "_${macro}.zip" $1 | sed -E 's/ .+//g')
        cat $2 | sed -E "s/__version__/${version}/g; s/__sha__/${sha}/g" > $outfile
        printf "$outfile created!\n"        
    fi

}
if [ -z $2 ]; then
    echo "Usage: bash tpl2rb.sh Jasspa_MicroEmacs_20240902-sha265.txt microemacs-spelling-template.tpl"
elif [ $1 == "lang2rb" ]; then   
    $1 "$@"
elif [ $1 == "mec2rb" ]; then   
    $1 "$@"
elif [ $1 == "emf2rb" ]; then   
    $1 "$@"
fi
