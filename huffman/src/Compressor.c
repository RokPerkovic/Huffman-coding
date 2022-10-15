/*
*	Koda za uporabniski vmesnik, ki omogoca uporabo Huffmanovega algoritma za kompresijo in dekompresijo in pa
*	Lempel-Ziv 77 algoritma za kompresijo in dekompresijo datotek.	
*/

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <omp.h>

GtkWidget *mainWindow;
GtkWidget *containerFixed;
GtkWidget *containerMenuBar;
GtkWidget *itemOpen;
GtkWidget *statusBarLabel;
GtkWidget *srcFileBtn;
GtkWidget *dstFileTxt;
GtkWidget *radBtnHuff;
GtkWidget *radBtn2;

GtkBuilder *builder;
char **globalArgs;
char *srcPath;
char dstPath[128];
char msg[128];
int algo = 1;


int main(int argc, char **args){
	globalArgs = args;

	gtk_init(&argc, &args);
		
	builder = gtk_builder_new_from_file("CompressorGUI.glade");
			
	mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
		
	g_signal_connect(mainWindow, "destroy", G_CALLBACK(gtk_main_quit),NULL);
		
	gtk_builder_connect_signals(builder, NULL);
		
	containerMenuBar = GTK_WIDGET(gtk_builder_get_object(builder, "containerMenuBar"));
	itemOpen = GTK_WIDGET(gtk_builder_get_object(builder, "itemOpen"));
	statusBarLabel = GTK_WIDGET(gtk_builder_get_object(builder, "statusBarLabel"));
	srcFileBtn = GTK_WIDGET(gtk_builder_get_object(builder, "srcFileBtn"));
	dstFileTxt = GTK_WIDGET(gtk_builder_get_object(builder, "dstFileTxt"));
	radBtnHuff = GTK_WIDGET(gtk_builder_get_object(builder, "radioBtnHuff"));
	radBtn2 = GTK_WIDGET(gtk_builder_get_object(builder, "radioBtn2"));
	
	gtk_entry_set_text(GTK_ENTRY(dstFileTxt), ".huff");
		
	gtk_widget_show(mainWindow);
		
	gtk_main();
	
	return 0;
}

void open_file_dialog();
void compress();
void decompress(char *);
double calcCompression(char *, char *);
void setMessage(char *);

//top menu bar
//menu items listeners
//activate --> click
//select --> hover
void on_itemOpen_activate(GtkMenuItem *item){
	open_file_dialog();
}

void on_srcFileBtn_file_set(GtkFileChooserButton *fbtn){
	srcPath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fbtn));
	gtk_label_set_text(GTK_LABEL(statusBarLabel), srcPath);
}



void open_file_dialog(){
	GtkWidget *dialog;
	int res;
	dialog = gtk_file_chooser_dialog_new("Choose file", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL);
	
	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if(res == GTK_RESPONSE_ACCEPT){
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		srcPath = gtk_file_chooser_get_filename(chooser);
		//printf("file choosen: %s\n", srcPath);
		gtk_label_set_text(GTK_LABEL(statusBarLabel), srcPath);
		//free(srcPath);
	}
	else{
		gtk_label_set_text(GTK_LABEL(statusBarLabel), "No file selected");
	}

	gtk_widget_destroy(dialog);
}

void on_cmprsButton_clicked(GtkButton *btn){
	
	if(srcPath == NULL){
		gtk_label_set_text(GTK_LABEL(statusBarLabel), "No file selected");
	}
	else{
		//gtk_label_set_text(GTK_LABEL(statusBarLabel), "Compressing..."); !!
		int rb = snprintf(dstPath, 127, "%s", gtk_entry_get_text(GTK_ENTRY(dstFileTxt))); //max dolzina imena stisnjene datoteke 127B.
		
		if(rb > 127){
			gtk_label_set_text(GTK_LABEL(statusBarLabel), "Maximum length for compressed file name is 127 characters!");
			return;	
		}
		else if(strlen(dstPath) == 0){
			gtk_label_set_text(GTK_LABEL(statusBarLabel), "Please specify compressed file name!");
			return;
		}
		//printf("dst filename: %s, len: %ld\n", dstPath, strlen(dstPath));
		double s = omp_get_wtime();
		compress(srcPath, algo);
		double e = omp_get_wtime();
		
		double cmpr = calcCompression(srcPath, dstPath) / 1000.0; 
		
		if(cmpr < 0){
			gtk_label_set_text(GTK_LABEL(statusBarLabel), "Done! Compressed file is bigger than original.");
		}
		else{
			sprintf(msg, "Done! Saved %f kB. Elapsed time: %f s", cmpr, e - s);
			gtk_label_set_text(GTK_LABEL(statusBarLabel), msg);
			
		}
	}
}

void on_dcmprsButton_clicked(GtkButton *btn){
	//TODO
	if(srcPath == NULL){
		gtk_label_set_text(GTK_LABEL(statusBarLabel), "No file selected");
	}
	
	else{
		gtk_label_set_text(GTK_LABEL(statusBarLabel), "Decompressing...");
		int rb = snprintf(dstPath, 127, "%s", gtk_entry_get_text(GTK_ENTRY(dstFileTxt))); //max dolzina imena razpihnjene datoteke 127B.
		
		if(rb > 127){
			gtk_label_set_text(GTK_LABEL(statusBarLabel), "Maximum length for decompressed file name is 127 characters!");
			return;	
		}
		//printf("dst filename: %s, len: %ld\n", dstPath, strlen(dstPath));
		
		double s = omp_get_wtime();
		decompress(srcPath);
		double e = omp_get_wtime();
		
		//gtk_label_set_text(GTK_LABEL(statusBarLabel), "Done!");
		sprintf(msg, "Done! Elapsed time: %f s", e - s);
		gtk_label_set_text(GTK_LABEL(statusBarLabel), msg);
	}
}


//TODO: autocomplete dst file extension based on algo selected
//algorithm radio buttons
void on_radioBtnHuff_toggled(GtkRadioButton *rb){
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb))){
		gtk_label_set_text(GTK_LABEL(statusBarLabel), "Huffman selected");
		algo = 1;
		gtk_entry_set_text(GTK_ENTRY(dstFileTxt), ".huff");
	}
}

void on_radioBtn2_toggled(GtkRadioButton *rb){
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb))){
		gtk_label_set_text(GTK_LABEL(statusBarLabel), "Lempel-Ziv 77 selected");
		algo = 2;
		gtk_entry_set_text(GTK_ENTRY(dstFileTxt), ".lz");
	}
}


void compress(char *fileName){
	pid_t pid;
	int statloc;
	globalArgs[1] = fileName;
	globalArgs[2] = dstPath;
	
	if((pid = fork()) < 0){
		perror("Fork error");
		exit(1);
	}
	else if(pid == 0){
		int e;
		if(algo == 1){
			e = execvp("./huffman/huff", globalArgs);
		}
		else if(algo == 2){
			//custom args za LZ
			char **lzArgs = malloc(5 * sizeof(char *));
			lzArgs[0] = "./lz77ppm\0";
			lzArgs[1] = "-stc\0";
			lzArgs[2] = fileName;
			lzArgs[3] = "-o\0"; 
			lzArgs[4] = dstPath;
			lzArgs[5] = NULL;

			e = execvp("./LZ/lz77/bin/lz77ppm", lzArgs);
			free(lzArgs[0]); free(lzArgs[1]); free(lzArgs[3]); free(lzArgs);	
		}
		
		//trenutni proces bo zamenjan s procesom drugega programa(cmd)
		if(e < 0){
			perror("couldn't execute compression.");
		}
		exit(127);
        }
        /*else{
        	gui zmrzne za cas kompresije
        	return;
        }*/
        
       if((pid = waitpid(pid, &statloc, 0)) < 0){
		perror("couldn`t wait");
		exit(1);
	}
}

void decompress(char *fileName){
	pid_t pid;
	int statloc;
	
	globalArgs[1] = fileName;
	globalArgs[2] = dstPath; 
	
	if((pid = fork()) < 0){
		perror("Fork error");
		exit(1);
	}
	else if(pid == 0){
		int e;
		if(algo == 1){
			e = execvp("./huffman/dcmprs", globalArgs);
		}
		else if(algo == 2){
			//custom args za LZ
			char **lzArgs = malloc(5 * sizeof(char *));
			lzArgs[0] = "./lz77ppm\0";
			lzArgs[1] = "-std\0";
			lzArgs[2] = fileName;
			lzArgs[3] = "-o\0"; 
			lzArgs[4] = dstPath;
			lzArgs[5] = NULL;

			e = execvp("./LZ/lz77/bin/lz77ppm", lzArgs);
			free(lzArgs[0]); free(lzArgs[1]); free(lzArgs[3]); free(lzArgs);
		}
		//trenutni proces bo zamenjan s procesom drugega programa(cmd)
		if(e < 0){
			perror("couldn't execute");
		}
		exit(127);
        }
        
        if((pid = waitpid(pid, &statloc, 0)) < 0){
		perror("couldn`t wait");
		exit(1);
	}
}


double calcCompression(char *srcPath, char *dstPath){
	struct stat fInfo;
	off_t srcfSize, dstfSize;
	
	if(stat(srcPath, &fInfo) < 0){
		perror("Could not retrieve source file info");
		gtk_label_set_text(GTK_LABEL(statusBarLabel), "Could not retrieve source file info.");	
	}
	srcfSize = fInfo.st_size; //izvorna
	
	if(stat(dstPath, &fInfo) < 0){
		dstPath = ".lz";
		if(stat(dstPath, &fInfo) < 0){
			perror("Could not retrieve srcFile info");
		}	
	}
	dstfSize = fInfo.st_size; //stisnjena
	
	off_t total = srcfSize - dstfSize;

	//printf("izvorna dat: %s, %d, stisnjena dat: %s, %d, saved: %d\n", srcPath, srcfSize, dstPath, dstfSize, total);
	
	return total;
}
