"""Lossless storage of backgrounds whose native pixels are exact 2x2 blocks."""
def expand_2x(data,width,height):
    assert len(data)==width*height*2
    out=bytearray(width*height*8)
    for y in range(height):
        row=bytearray()
        for x in range(width):row+=data[(y*width+x)*2:(y*width+x+1)*2]*2
        at=y*2*width*4;out[at:at+len(row)]=row;out[at+len(row):at+len(row)*2]=row
    return bytes(out)
def fold_2x(data,width,height):
    assert not width%2 and not height%2 and len(data)==width*height*2
    packed=b''.join(data[(y*width+x)*2:(y*width+x+1)*2] for y in range(0,height,2) for x in range(0,width,2))
    assert expand_2x(packed,width//2,height//2)==data,'Background is not made of exact 2x2 blocks'
    return packed
