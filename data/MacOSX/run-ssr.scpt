FasdUAS 1.101.10   ��   ��    k             l     ��  ��    ) # To store last chosen renderer type     � 	 	 F   T o   s t o r e   l a s t   c h o s e n   r e n d e r e r   t y p e   
  
 j     �� �� &0 previousselection previousSelection  m     ����       l     ��������  ��  ��     ��  i        I     �� ��
�� .aevtoappnull  �   � ****  o      ���� 0 argv  ��    k    3       l     ��  ��    / ) Get absolute path to SSR binary via args     �   R   G e t   a b s o l u t e   p a t h   t o   S S R   b i n a r y   v i a   a r g s      l     ��  ��    \ V For relative paths to work, the binary must be placed in MacOS/ folder of .app bundle     �   �   F o r   r e l a t i v e   p a t h s   t o   w o r k ,   t h e   b i n a r y   m u s t   b e   p l a c e d   i n   M a c O S /   f o l d e r   o f   . a p p   b u n d l e      r        !   n      " # " 4   �� $
�� 
cobj $ m    ����  # o     ���� 0 argv   ! o      ���� 0 
ssrbinpath 
ssrBinPath   % & % r     ' ( ' n    
 ) * ) 1    
��
�� 
rest * o    ���� 0 argv   ( o      ���� 0 argv   &  + , + l   ��������  ��  ��   ,  - . - l   �� / 0��   / G A Make list of renderer type names and their parameter equivalents    0 � 1 1 �   M a k e   l i s t   o f   r e n d e r e r   t y p e   n a m e s   a n d   t h e i r   p a r a m e t e r   e q u i v a l e n t s .  2 3 2 r     4 5 4 J     6 6  7 8 7 m     9 9 � : : , B i n a u r a l   ( u s i n g   H R I R s ) 8  ; < ; m     = = � > > J B i n a u r a l   R o o m   S y n t h e s i s   ( u s i n g   B R I R s ) <  ? @ ? m     A A � B B ( W a v e   F i e l d   S y n t h e s i s @  C D C m     E E � F F 8 A m b i s o n i c s   A m p l i t u d e   P a n n i n g D  G H G m     I I � J J X S t e r e o p h o n i c   ( V e c t o r   B a s e   A m p l i t u d e   P a n n i n g ) H  K�� K m     L L � M M   G e n e r i c   R e n d e r e r��   5 o      ���� 0 renderernames rendererNames 3  N O N r    & P Q P J    " R R  S T S m     U U � V V  - - b i n a u r a l T  W X W m     Y Y � Z Z 
 - - b r s X  [ \ [ m     ] ] � ^ ^ 
 - - w f s \  _ ` _ m     a a � b b 
 - - a a p `  c d c m     e e � f f  - - v b a p d  g�� g m      h h � i i  - - g e n e r i c��   Q o      ���� "0 rendereroptions rendererOptions O  j k j l  ' '��������  ��  ��   k  l m l l  ' '�� n o��   n / ) Process other given command line options    o � p p R   P r o c e s s   o t h e r   g i v e n   c o m m a n d   l i n e   o p t i o n s m  q r q r   ' , s t s m   ' (��
�� boovtrue t o      ���� $0 pickrenderertype pickRendererType r  u v u r   - 8 w x w c   - 4 y z y m   - 0 { { � | |   z m   0 3��
�� 
utxt x o      ���� 0 options   v  } ~ } X   9 o �� �  k   K j � �  � � � r   K X � � � b   K T � � � b   K R � � � o   K N���� 0 options   � m   N Q � � � � �    � o   R S���� 0 arg   � o      ���� 0 options   �  ��� � Z   Y j � ����� � E  Y ^ � � � o   Y \���� "0 rendereroptions rendererOptions � o   \ ]���� 0 arg   � r   a f � � � m   a b��
�� boovfals � o      ���� $0 pickrenderertype pickRendererType��  ��  ��  �� 0 arg   � o   < =���� 0 argv   ~  � � � l  p p��������  ��  ��   �  � � � l  p p�� � ���   � < 6 Let user pick renderer type if none was given in argv    � � � � l   L e t   u s e r   p i c k   r e n d e r e r   t y p e   i f   n o n e   w a s   g i v e n   i n   a r g v �  � � � r   p { � � � c   p w � � � m   p s � � � � �   � m   s v��
�� 
utxt � o      ����  0 rendereroption rendererOption �  � � � Z   |  � ����� � l  |  ����� � o   | ���� $0 pickrenderertype pickRendererType��  ��   � k   � � �  � � � Z   � � � ����� � ?  � � � � � o   � ����� &0 previousselection previousSelection � l  � � ����� � I  � ��� ���
�� .corecnte****       **** � o   � ����� 0 renderernames rendererNames��  ��  ��   � r   � � � � � m   � �����  � o      ���� &0 previousselection previousSelection��  ��   �  � � � O   � � � � � k   � � � �  � � � I  � �������
�� .miscactvnull��� ��� null��  ��   �  ��� � r   � � � � � I  � ��� � �
�� .gtqpchltns    @   @ ns   � o   � ����� 0 renderernames rendererNames � �� � �
�� 
appr � m   � � � � � � � 2 S t a r t   S o u n d S c a p e   R e n d e r e r � �� � �
�� 
prmp � m   � � � � � � � � P l e a s e   m a k e   s u r e   J a c k   i s   r u n n i n g .   S e l e c t   t h e   t y p e   o f   r e n d e r e r   f o r   t h i s   s e s s i o n : � �� ���
�� 
inSL � J   � � � �  ��� � n   � � � � � 4   � ��� �
�� 
cobj � o   � ����� &0 previousselection previousSelection � o   � ����� 0 renderernames rendererNames��  ��   � o      ���� ,0 selectedrenderername selectedRendererName��   � m   � � � ��                                                                                  sevs  alis    �  Macintosh HD               ƫ�H+     jSystem Events.app                                               *��8CW        ����  	                CoreServices    ƫ{�      �8'7       j   '   &  :Macintosh HD:System:Library:CoreServices:System Events.app  $  S y s t e m   E v e n t s . a p p    M a c i n t o s h   H D  -System/Library/CoreServices/System Events.app   / ��   �  ��� � Z   � � ��� � � =  � � � � � o   � ����� ,0 selectedrenderername selectedRendererName � m   � ���
�� boovfals � L   � �����  ��   � k   � � �  � � � r   � � � � � n   � � � � � 4  � ��� �
�� 
cobj � m   � �����  � o   � ����� ,0 selectedrenderername selectedRendererName � o      ���� ,0 selectedrenderername selectedRendererName �  ��� � Y   � ��� � ��� � Z   � � ����� � =  � � � � � o   � ����� ,0 selectedrenderername selectedRendererName � l  � � ����� � n   � � � � � 4   � ��� �
�� 
cobj � o   � ����� 0 i   � o   � ����� 0 renderernames rendererNames��  ��   � k   � � �  � � � r   �	 � � � n   � � � � 4  �� �
�� 
cobj � o  ���� 0 i   � o   ����� "0 rendereroptions rendererOptions � o      ����  0 rendereroption rendererOption �  � � � l 

�� � ���   �   Save user choice     � � � � $   S a v e   u s e r   c h o i c e   �  � � � r  
 � � � o  
���� 0 i   � o      ���� &0 previousselection previousSelection �  ��� �  S  ��  ��  ��  �� 0 i   � m   � �����  � l  � � ����� � I  � ��� ���
�� .corecnte****       **** � n  � � � � � 2  � ���
�� 
cobj � o   � ����� 0 renderernames rendererNames��  ��  ��  ��  ��  ��  ��  ��   �  � � � l !!��������  ��  ��   �  �  � l !!����   $  Assemble shell command string    � <   A s s e m b l e   s h e l l   c o m m a n d   s t r i n g   r  !F b  !B	 b  !>

 b  !: b  !6 b  !2 b  !0 b  !, b  !* b  !& m  !$ � " e x p o r t   E C A S O U N D = " o  $%���� 0 
ssrbinpath 
ssrBinPath m  &) � " / e c a s o u n d "   ;   c d   " o  *+���� 0 
ssrbinpath 
ssrBinPath m  ,/ �  / . . / . . / . . "   ;   " o  01���� 0 
ssrbinpath 
ssrBinPath m  25   �!!  / s s r "   o  69����  0 rendereroption rendererOption o  :=���� 0 options  	 m  >A"" �## �   & &   ( e c h o   S S R   q u i t   n o r m a l l y   w i t h   e x i t   c o d e   $ ? )   | |   ( e c h o   S S R   e n c o u n t e r e d   a n   e r r o r   a n d   h a d   t o   q u i t .   S e e   a b o v e .     E x i t   c o d e :   $ ? ) o      �� 0 command   $%$ l GG�~�}�|�~  �}  �|  % &'& l GG�{()�{  ( D > Open new Terminal window, cd to SSR working dir and start SSR   ) �** |   O p e n   n e w   T e r m i n a l   w i n d o w ,   c d   t o   S S R   w o r k i n g   d i r   a n d   s t a r t   S S R' +,+ O G^-.- r  M]/0/ l MY1�z�y1 I MY�x2�w
�x .coredoexbool       obj 2 4  MU�v3
�v 
prcs3 m  QT44 �55  T e r m i n a l�w  �z  �y  0 o      �u�u "0 terminalrunning terminalRunning. m  GJ66�                                                                                  sevs  alis    �  Macintosh HD               ƫ�H+     jSystem Events.app                                               *��8CW        ����  	                CoreServices    ƫ{�      �8'7       j   '   &  :Macintosh HD:System:Library:CoreServices:System Events.app  $  S y s t e m   E v e n t s . a p p    M a c i n t o s h   H D  -System/Library/CoreServices/System Events.app   / ��  , 787 O  _19:9 k  e0;; <=< Z  e�>?�t@> o  eh�s�s "0 terminalrunning terminalRunning? k  k|AA BCB I kp�r�q�p
�r .miscactvnull��� ��� null�q  �p  C D�oD r  q|EFE I qx�nG�m
�n .coredoscnull��� ��� ctxtG o  qt�l�l 0 command  �m  F o      �k�k 0 ssrtab ssrTab�o  �t  @ l �HIJH k  �KK LML I ��j�i�h
�j .miscactvnull��� ��� null�i  �h  M N�gN r  ��OPO I ���fQR
�f .coredoscnull��� ��� ctxtQ o  ���e�e 0 command  R �dS�c
�d 
kfilS 4 ���bT
�b 
cwinT m  ���a�a �c  P o      �`�` 0 ssrtab ssrTab�g  I F @ Terminal not running yet. Start it and use fresh default window   J �UU �   T e r m i n a l   n o t   r u n n i n g   y e t .   S t a r t   i t   a n d   u s e   f r e s h   d e f a u l t   w i n d o w= VWV l ���_�^�]�_  �^  �]  W XYX l ���\Z[�\  Z 4 . Position Terminal window in upper left corner   [ �\\ \   P o s i t i o n   T e r m i n a l   w i n d o w   i n   u p p e r   l e f t   c o r n e rY ]^] r  ��_`_ J  ��aa bcb m  ���[�[  c d�Zd m  ���Y�Y  �Z  ` n      efe 1  ���X
�X 
pposf 4 ���Wg
�W 
cwing m  ���V�V ^ hih l ���U�T�S�U  �T  �S  i jkj l ���Rlm�R  l   Wait until SSR quits   m �nn *   W a i t   u n t i l   S S R   q u i t sk opo I ���Qq�P
�Q .sysodelanull��� ��� nmbrq m  ���O�O �P  p rsr W  ��tut I ���Nv�M
�N .sysodelanull��� ��� nmbrv m  ���L�L �M  u l ��w�K�Jw E  ��xyx n ��z{z 1  ���I
�I 
hist{ o  ���H�H 0 ssrtab ssrTaby o  ���G�G 0 command  �K  �J  s |}| V  ��~~ I ���F��E
�F .sysodelanull��� ��� nmbr� m  ���D�D �E   l ����C�B� = ����� n  ����� 1  ���A
�A 
busy� o  ���@�@ 0 ssrtab ssrTab� m  ���?
�? boovtrue�C  �B  } ��� l ���>�=�<�>  �=  �<  � ��� l ���;���;  � ? 9 Close Terminal window if SSR did quit with exit code 0		   � ��� r   C l o s e   T e r m i n a l   w i n d o w   i f   S S R   d i d   q u i t   w i t h   e x i t   c o d e   0 	 	� ��:� P  �0��9�� Z  �/���8�7� F  �	��� l ����6�5� E  ����� n ����� 1  ���4
�4 
hist� o  ���3�3 0 ssrtab ssrTab� m  ���� ��� D S S R   q u i t   n o r m a l l y   w i t h   e x i t   c o d e   0�6  �5  � l ���2�1� H  ��� E  ���� n � ��� 1  � �0
�0 
hist� o  ���/�/ 0 ssrtab ssrTab� m   �� ���  - - h e l p�2  �1  � I +�.��
�. .coreclosnull���     obj � l !��-�,� 6 !��� 4 �+�
�+ 
cwin� m  �*�* � =  ��� 1  �)
�) 
tcnt� o  �(�( 0 ssrtab ssrTab�-  �,  � �'��&
�' 
savo� m  $'�%
�% savono  �&  �8  �7  �9  � �$�#
�$ conscase�#  �:  : m  _b���                                                                                      @ alis    j  Macintosh HD               ƫ�H+     sTerminal.app                                                     X?�wc�        ����  	                	Utilities     ƫ{�      �wG�       s   r  0Macintosh HD:Applications:Utilities:Terminal.app    T e r m i n a l . a p p    M a c i n t o s h   H D  #Applications/Utilities/Terminal.app   / ��  8 ��"� l 22�!� ��!  �   �  �"  ��       �����  � ��� &0 previousselection previousSelection
� .aevtoappnull  �   � ****� � � �����
� .aevtoappnull  �   � ****� 0 argv  �  � ���� 0 argv  � 0 arg  � 0 i  � @��� 9 = A E I L�� U Y ] a e h�� {���
�	 � �� ��� �� ���� "�� 4�����������������������������������
� 
cobj� 0 
ssrbinpath 
ssrBinPath
� 
rest� � 0 renderernames rendererNames� "0 rendereroptions rendererOptions� $0 pickrenderertype pickRendererType
� 
utxt� 0 options  
�
 
kocl
�	 .corecnte****       ****�  0 rendereroption rendererOption
� .miscactvnull��� ��� null
� 
appr
� 
prmp
� 
inSL
� .gtqpchltns    @   @ ns  � ,0 selectedrenderername selectedRendererName� 0 command  
�  
prcs
�� .coredoexbool       obj �� "0 terminalrunning terminalRunning
�� .coredoscnull��� ��� ctxt�� 0 ssrtab ssrTab
�� 
kfil
�� 
cwin
�� 
ppos
�� .sysodelanull��� ��� nmbr
�� 
hist
�� 
busy
�� 
bool�  
�� 
tcnt
�� 
savo
�� savono  
�� .coreclosnull���     obj �4��k/E�O��,E�O�������vE�O�����a �vE` OeE` Oa a &E` O 5�[a �l kh _ a %�%E` O_ � 
fE` Y h[OY��Oa a &E` O_  �b   �j  kEc   Y hOa  **j O�a a a a  a !��b   /kv� "E` #UO_ #f  hY E_ #�k/E` #O 7k��-j kh _ #��/  _ �/E` O�Ec   OY h[OY��Y hOa $�%a %%�%a &%�%a '%_ %_ %a (%E` )Oa  *a *a +/j ,E` -UOa . �_ - *j O_ )j /E` 0Y *j O_ )a 1*a 2k/l /E` 0Ojjlv*a 2k/a 3,FOlj 4O h_ 0a 5,_ )kj 4[OY��O h_ 0a 6,e kj 4[OY��Oga 7 G_ 0a 5,a 8	 _ 0a 5,a 9a :& $*a 2k/a ;[a <,\Z_ 081a =a >l ?Y hVUOP ascr  ��ޭ