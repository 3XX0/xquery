<root>{
for $s in doc(j_caesar.xml)//SPEAKER
return <speaks>{
		<who>{$s/text()}</who>,
                for $a in doc(j_caesar.xml)//ACT
                where some $s1 in ($a//SPEAKER) satisfies ($s1 eq $s)
                return <when>{$a/TITLE/text()}</when>
	}</speaks>
}</root>
