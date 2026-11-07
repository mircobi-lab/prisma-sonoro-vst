import numpy as np, scipy.io.wavfile as w, scipy.signal as sg
def load(n):
    sr,x=w.read(f'out/{n}.wav'); return sr,x[:,0].astype(float),x[:,1].astype(float)
def peakf(x,sr,lo=1,hi=20000,n=1):
    X=np.abs(np.fft.rfft(x*np.hanning(len(x)),len(x)*4)); f=np.fft.rfftfreq(len(x)*4,1/sr)
    m=(f>lo)&(f<hi); idx=sg.find_peaks(X*m,distance=20)[0]; idx=idx[np.argsort(X[idx])[::-1][:n]]
    return sorted([(round(f[i],2),round(20*np.log10(X[i]/X.max()),1)) for i in idx])
sr,L,R=load('01_sine_528_fixed'); print('01 sine:',peakf(L[sr:],sr))
sr,L,R=load('02_binaural_200_783'); print('02 binaural L:',peakf(L[sr:],sr),' R:',peakf(R[sr:],sr))
sr,L,R=load('03_iso_400_10'); env=np.abs(sg.hilbert(L[sr:])); E=np.abs(np.fft.rfft(env-env.mean())); fE=np.fft.rfftfreq(len(env),1/sr); print('03 iso: ritmo impulsi %.2f Hz'%fE[np.argmax(E[1:200])+1])
sr,L,R=load('04_saw_A6_alias'); x=L[int(.2*sr):int(1.5*sr)]; X=np.abs(np.fft.rfft(x*sg.windows.blackmanharris(len(x)))); f=np.fft.rfftfreq(len(x),1/sr)
harm=np.zeros_like(X,bool)
for k in range(1,14): harm|=np.abs(f-1760*k)<12
print('04 saw 1760 Hz: armonici sotto 21,6k: %d; energia fuori armonici %.1f dB'%(int(21600/1760),10*np.log10(X[~harm].__pow__(2).sum()/X[harm].__pow__(2).sum())))
sr,L,R=load('05_bowl_236'); print('05 campana 236:',peakf(L[:4*sr],sr,100,4000,6))
seg=L[int(.5*sr):int(6*sr)]; env=np.abs(sg.hilbert(sg.sosfilt(sg.butter(4,[220,252],'bp',fs=sr,output='sos'),seg)))
E=np.abs(np.fft.rfft(env-env.mean())); fE=np.fft.rfftfreq(len(env),1/sr); print('   battimento del doppietto fondamentale: %.2f Hz (atteso %.2f)'%(fE[np.argmax(E[2:400])+2],236*0.008))
sr,L,R=load('06_gong_om_13610'); print('06 gong: picco dominante',peakf(L[:3*sr],sr,60,400,1))
sr,L,R=load('07_vibes_A4_trem4'); print('07 vibrafono:',peakf(L[:2*sr],sr,100,6000,3)); env=np.abs(sg.hilbert(sg.sosfilt(sg.butter(4,[400,480],'bp',fs=sr,output='sos'),L[:2*sr])))
E=np.abs(np.fft.rfft(env-np.convolve(env,np.ones(4800)/4800,'same'))); fE=np.fft.rfftfreq(len(env),1/sr); print('   tremolo %.2f Hz'%fE[np.argmax(E[5:200])+5])
sr,L,R=load('08_didge_D2'); print('08 didgeridoo:',peakf(L[sr:],sr,40,4000,6))
sr,L,R=load('09_rings_432'); print('09 anelli:',peakf(L[2*sr:],sr,300,4000,7),' attesi',[round(432*7/k,1) for k in range(7,0,-1)])
sr,L,R=load('10_drawn_110'); per=sr/110; seg=L[sr:sr+int(per*4)]
cyc=np.array([ (np.sin(x/0.7*np.pi) if x<0.7 else -np.sin((x-0.7)/0.3*np.pi)*0.6)+0.15*np.sin(x*2*np.pi*9) for x in np.arange(256)/256])
ref=np.interp(np.arange(len(seg))/per%1*256,np.arange(256),cyc); ref-=ref.mean()
best=max(np.corrcoef(np.roll(seg,s),ref)[0,1] for s in range(0,int(per),2)); print('10 forma disegnata: correlazione con la forma originale %.4f'%best)
sr,L,R=load('11_sweep_55_880_6s'); a=sg.hilbert(L); ph=np.unwrap(np.angle(a)); fi=np.diff(ph)*sr/(2*np.pi)
for t in [0.5,3,5.5]: i=int(t*sr); print('11 sweep t=%.1fs: %.1f Hz (atteso %.1f)'%(t,np.median(fi[i-200:i+200]),55*16**(t/6)))
sr,L,R=load('12_grey_noise'); f,P=sg.welch(L,sr,nperseg=8192); band=lambda a,b:10*np.log10(P[(f>a)&(f<b)].mean())
print('12 grigio: 100 Hz %.1f, 1 kHz %.1f, 3,5 kHz %.1f, 12 kHz %.1f dB'%(band(80,120),band(900,1100),band(3300,3700),band(11000,13000)))
sr,L,R=load('13_sub_783_tone'); print('13 sub 7,83 Hz picco %.3f (attenuato)'%np.abs(L).max())
sr,L,R=load('14_poly_chord_limiter'); print('14 limitatore picco %.4f (tetto 0,891) — dB %.2f'%(np.abs(np.r_[L,R]).max(),20*np.log10(np.abs(np.r_[L,R]).max())))
