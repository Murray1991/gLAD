
var new_file = "very_little_queries.txt";
var num_lines = 126214;
var thresh = 500;

function random () {
    return Math.random() * num_lines;
}

var lineReader = require('readline').createInterface({
    input: require('fs').createReadStream('medium_queries.txt')
});


var fs = require('fs');
lineReader.on('line', function (line) {
    if ( random() < thresh ) {
        var str = line;
        //var size = str.length;
        //if ( size > 1 )
        //    size = size/2;
        var newstr = str //str.substring(0,size);
        fs.appendFile(new_file, newstr+"\n");
    }
});
