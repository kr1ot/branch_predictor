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

#run 5
./sim gshare 9 3 gcc_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_gshare_1.txt

#run 6
./sim gshare 14 8 gcc_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_gshare_2.txt

#run 7
./sim gshare 11 5 jpeg_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_gshare_3.txt

#run 8
./sim gshare 10 6 perl_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_gshare_4.txt

#run 9
./sim hybrid 8 14 10 5 gcc_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_hybrid_1.txt

#run 10
./sim hybrid 5 10 7 5 jpeg_trace.txt > output.txt
diff -iuw output.txt proj2-validation/val_hybrid_2.txt
