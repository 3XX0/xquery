for $b in doc(input_join_books.xml)/book,
    $a in doc(input_join_reviews.xml)/entry,
    $c in $a,
    $tb in $c/title,
    $ta in $c/title

    where $tb eq $ta

    return <bookwithprices>{
                $tb,
                <pricereview>{
                        $a/price/text()
                }</pricereview>,
                <price>{
                        $b/price/text()
                }</price>
           }</bookwithprices>
