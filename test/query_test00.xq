<result>{
for $a in doc(j_caesar.xml)//ACT,
    $sc in $a//SCENE,
    $sp in $sc/SPEECH
where $sp/LINE/text() = "Et tu, Brute! Then fall, Caesar."
return <who>{
                $sp/SPEAKER/text()
        }</who>,
        <when>{
                <act>{
                        $a/TITLE/text()
                }</act>,
                <scene>{
                        $sc/TITLE/text()
                }</scene>
        }</when>
}</result>
