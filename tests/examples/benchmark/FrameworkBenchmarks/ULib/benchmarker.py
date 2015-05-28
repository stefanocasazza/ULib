      p = subprocess.Popen("sudo -s ulimit -a".rsplit(" "), stdout=subprocess.PIPE)
      out = p.communicate()
      print "sudo -s ulimit -a:\n %s" % (out,)
