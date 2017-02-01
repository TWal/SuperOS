print (".text")
for i in range (256):
        print ("_inter_%d:\n  push %%edi\n  mov $%d,%%edi\n  jmp geneInt\n\n" % (i,4*i))
print ("\n\ninitIntIDT:\n")
print ("  .global initIntIDT")
for i in range(256):
        print ("  movl $_inter_%d,intIDT + %d" % (i,4*i))
print ("  ret")
