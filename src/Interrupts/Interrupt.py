print (".text")
for i in range (256):
        print ("_inter_%d:\n  push %%rbx\n  mov $%d,%%rbx\n  jmp geneInt\n\n" % (i,i))
        #print ("_inter_%d:\n  push %%edi\n  mov $%d,%%edi\n  cli\n  hlt\n\n" % (i,i))
print ("\n\ninitIntIDT:\n")
print ("  .global initIntIDT")
print ("  movabs $intIDT,%r10")
for i in range(256):
        print ("  movabs $_inter_%d, %%r11\n  movq %%r11,%d(%%r10)" % (i,8*i))
print ("  ret")
