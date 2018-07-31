NAME = llvm-CIA

dist:
	rm -rf $(NAME)
	rsync -aR --copy-unsafe-links `cat MANIFEST` $(NAME)
	tar cvf - $(NAME) | gzip -vf9 > $(NAME).tar.gz
	rm -rf $(NAME)
