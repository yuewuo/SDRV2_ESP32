with open('weuimy2.css') as f:
    with open('weuimy.css', 'w') as o:
        st = f.read()
        for c in st:
            if c != '\n':
                o.write(c)