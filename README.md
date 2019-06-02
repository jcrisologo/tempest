# TEMPEST Transmission

> Jove's lightnings, the precursors  
> O' the dreadful thunder-claps, more momentary  
> And sight-outrunning were not; the fire and cracks  
> Of sulphurous roaring the most mighty Neptune  
> Seem to besiege and make his bold waves tremble,  
> Yea, his dread trident shake. (*The Tempest* 1.2.320-325)

Actually, this project has nothing to do with Shakespeare, storms, seas, stormy seas or really 
anything else that naturally comes to mind when thinking about tempests.

## About

[TEMPEST](https://en.wikipedia.org/wiki/Tempest_(codename)) is the NSA codeword for a particular class of information-leaking emission
produced by electronic devices.  Maxwell's equations tell us that a change in current produces a change in magnetic field.  A change in magnetic field induces a change in electric field and vice versa.  Thus we obtain the self-propagating electromagnetic radiation that so much of our modern-day technological communication depends on.  But ALL of our electronic have changing currents, whether we want to propagate radiation or not.  Extra processing power during a cryptographic action will cause it.  Changing a pixel's brightness too.  The emissions gererated by these events can be captured and analyzed.

First discovered by Bell Labs during World War II, this phenomena is now well understood and well defended against — at least for the devices that the NSA cares about.  Consumer devices tend to not have the same level of protection, thus this project is doable on a budget of breadcrumbs.

TEMPEST is generally assumed to only be a passive attack by an eavesdropper with no control over the target platform.  TEMPEST Transmission
explores the idea of turning this into an active attack: what havoc can a malicious actor wreak with direct access to the target?  (Gettng access is an exercise left to the reader.  Insider threats, social engineering, and supply-side attacks against display driver distributors are all viable avenues for attack.)

TEMPEST Transmission uses the TEMPEST emissions of a monitor to convey information.  It first demonstrates generation of an AM carrier signal modulated with music.  After showing sufficient control over the TEMPEST signal, it moves on to transmitting OOK, BPSK, QPSK and DQPSK modulated digital data at a rate of one symbol per display frame.  The signal is captured by a USRP and demodulated in Simulink, however consumer radios are perfectly capable of receiving and playing the AM signal.

## Prior Works

This project would have seemed like a hopeless effort were the path not already forged by the following:

- [Tempest for Eliza](http://www.erikyyy.de/tempest/) Heavily based on this starter code.
- [Soft Tempest](http://www.cl.cam.ac.uk/~mgk25/ih98-tempest.pdf)
- [Airhopper](https://arxiv.org/abs/1411.0237)

## Environment
Tested only on Lubuntu 18.10

```sudo apt-get install ffmpeg sox libsdl2-dev libliquid-dev libconfig-dev git build-essential```

Most development was done using an ASUS VH242 monitor conencted over VGA.  TEMPEST for digital signals will likely require an entirely different strategy to what is done here.  The phenomena was reproduced on an ASUS UX301LA laptop.  A third device, the ASUS eeePC 1005HA did not have the processing power (Intel Atom N280) to meet its own screen's 60 Hz refresh rate.

## Build
```make all```

## Usage
```./tempest_transmission config_file data_file```
