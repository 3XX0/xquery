for $b in doc(input_join_books.xml)/book,
    $a in doc(input_join_reviews.xml)/entry,
    $tb in $b/title,
    $ta in $a/title

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
