make clean
make sim

#run 1
./sim bimodal 6 gcc_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_bimodal_1.txt

#run 2
./sim bimodal 12 gcc_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_bimodal_2.txt

#run 3
./sim bimodal 4 jpeg_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_bimodal_3.txt

#run 4
./sim bimodal 5 perl_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_bimodal_4.txt

