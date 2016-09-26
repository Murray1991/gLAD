
var new_file = "medium_queries.txt";
var num_lines = 2126355;
var thresh = 2000000;

function random () {
    return Math.random() * (2126354);
}

var lineReader = require('readline').createInterface({
    input: require('fs').createReadStream('big_queries.txt')
});


var fs = require('fs');
lineReader.on('line', function (line) {
    if ( random() > thresh ) {
        fs.appendFile(new_file, line+"\n");
    }
});
