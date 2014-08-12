for file in $( ls *.1 ); do ls -d ../../../lsf/cmdtools/${file%.1} ; done
for file in $( ls *.1 ); do if [[ ! -d ../../../lsf/cmdtools/${file%.1} ]]; then mkdir ../../../lsf/cmdtools/${file%.1} ; fi ; done
