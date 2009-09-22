/*
* Copyright (c) 2009 David Roberts <d@vidr.cc>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

var PC = 0;
function sq(input_buffer) {
    if(PC == 0) input_buffer = '';
    input_buffer = input_buffer.split('');
    input_buffer.reverse();
    var output_buffer = '';
    while(PC >= 0) {
        var a = mem[PC+0];
        var b = mem[PC+1];
        var c = mem[PC+2];
        
        if(a < 0) { // input
            if(input_buffer.length == 0)
                break;
            if(mem[b] == undefined) mem[b] = 0;
            mem[b] += input_buffer.pop().charCodeAt(0); 
            PC += 3;
        } else if(b < 0) { // output
            if(mem[a] == undefined) mem[a] = 0;
            output_buffer += String.fromCharCode(mem[a]);
            PC += 3;
        } else {
            if(mem[a] == undefined) mem[a] = 0;
            if(mem[b] == undefined) mem[b] = 0;
            mem[b] -= mem[a];
            if(mem[b] <= 0) PC = c;
            else            PC += 3;
        }
        
        if(output_buffer.length > 256) {
            output_buffer += '... HALTED due to excessive output';
            output_buffer += ' (Refresh page to reset)\n';
            PC = -1;
            break;
        } else if(PC < 0) {
            output_buffer += '\nHALTED (Refresh page to reset)\n';
            break;
        }
    }
    return output_buffer;
}

function jsubleq(container, focus) {
    var terminal = $(container)
        .append('<div id="jsubleqterminal"></div>').find('div:last');
    var output_span = terminal
        .append('<span></span>').find('span:last');
    var form = terminal
        .append('<form id="jsubleqform" onsubmit="return false;"></form>')
        .find('form:last');
    var input = form
        .append('<input id="jsubleqinput" type="text" name="prompt" />')
        .find('input:last');
    
    var output = function(text) {
        text = $('<span></span>').text(text).html().replace(/\n/g, '<br />');
        output_span.append('<span>' + text + '</span>');
    }
    
    form.submit(function() {
        var val = $.trim(input.val())+'\n';
        form.hide();
        output(val);
        output(sq(val));
        form.show();
        input.val('').focus();
        if(PC < 0) input.blur().attr('disabled', 'disabled');
    });

    $(document).ready(function() {
        $(container).mouseup(function() {
            if(document.getSelection() == '') input.focus();});
        output(sq());
        if(focus) input.focus();
    });
}
