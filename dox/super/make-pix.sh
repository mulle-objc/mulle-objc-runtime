#! /bin/sh

pixtype="${1:-png}"

for i in *.dot
do
   pix="`basename "$i" .dot`.${pixtype}"
   echo "$i -> $pix" >&2
   if ! dot -T${pixtype} "$i" > "$pix"
   then
      rm "$pix"
      exit 1
   fi
done

