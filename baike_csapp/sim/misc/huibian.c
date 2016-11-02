

int dx = 1;
int ax = 800;
int c;
c = dx;
dx = *ax;
*ax = c;

// jump 

int sp = 0x7f0;
int ax = 0;

if(dx == 0)
	sender();
else
	receiver();

/*______________Receriver Part____________*/

sender(){
	dx = 0; //conter
	int bx = 1;
	senderbegin()
}

senderbegin(){
	int si = S[0x800];
	if(s[0x800]!=0) // this part must be set in the simulater !!!
		senderbegin();
	else
		int cx = dx;
}
senderreading(){
	if(cx==0)
		SenderInter();
}

SenderInter(){
	dx = dx + 1;
	cx = dx;
	ax = ax + cx;
	SenderPutting();
}

d c a
0 0 0 
1 1 1

1 0 0
1 -1 -1
1 -2 -2
...

SenderPutting(){
	if(cx == 0){
		SenderEnd()
	}
	else{
		Put();
		ax = ax - 1;
		cx = cx - 1;
		SenderPutting();  
	}
}

Put(){
	int di = 4096; //(conter)0x 1000
	di = di+4;
	s[di+4096] = ax;
	s[4086] = di+1000; // address
}

SenderEnd(){
	s[800] = bx;
	dx = dx + 1;
	if(dx >= 100)
		halt;
	else
		senderbegin();
}


/*______________Read part _____________*/
Read(){
	r[4096] = di;
	if(di = 0)
		return 
	else{
		r[4096+di] = ax;
		di = di - 4;
		r[4096] = di;
	}
	return;
}
receiver(){
	dx = 1; //counter?
	bx = 0;
	t[800]=bx;
	receiverbegin();
}
receiverbegin(){
	si = t[800];
	if(si ==0 )
		receiverbegin();
	else{
		cx = dx;
	}
}
receriverreading(){
	if(cx == 0)
		receiveinter();
	else{
		Read();
		cx = cx -1;
		receriverreading();
	}
}
receiveinter(){
	dx = dx + 1;
	cx = dx;
	ax = ax + dx;
}

receriverput(){
	if(cx ==0)
		receiveend(){}
}

receiveend(){
	s[800] = ebx;
	if(edx>=100)
		halt;
	else{
		edx = edx + 1;
	}
}

Halt;
RET;