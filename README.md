# System Programming Lab 11 Multiprocessing

## Discussion of results
My runtime decreases when adding more processes until 20 processes. This is most likely because the compiler gives more images to one process than the others causing it to go longer as it has to switch off and complete more images itself.

## Thread Implementation
My implementation uses most of the parts of compute image to split the images apart for the multiple threads to share the processing but keep the data together unlike the multiprocessing that just splits up the amount of images between each other.

## Processes and Threads Table
All runtime is in seconds.
![image](https://github.com/user-attachments/assets/91cfdd00-3aab-4d32-b24e-710a2e06fd38)

