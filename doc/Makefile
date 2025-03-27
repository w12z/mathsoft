# 把该文件放入相应的目录下，然后在终端输入 make 命令即可编译
# 你的作业应该能通过编译，否则请检查你的代码

# 作业文件名
PROJ = report

all: $(PROJ).pdf


# 没有参考文献用这个
$(PROJ).pdf: $(PROJ).tex
	xelatex $(PROJ).tex


# 有参考文献用这个，并注释掉上面的
#$(PROJ).pdf: $(PROJ).tex
#	xelatex $(PROJ).tex
#	bibtex $(PROJ)
#	xelatex $(PROJ).tex
#	xelatex $(PROJ).tex


clean:
	rm -f *.aux *.log *.out *.toc *.bbl *.blg *.lof *.lot *.pdf