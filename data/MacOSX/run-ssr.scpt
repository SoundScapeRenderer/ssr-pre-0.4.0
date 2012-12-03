FasdUAS 1.101.10   ��   ��    k             l     ��  ��    ) # To store last chosen renderer type     � 	 	 F   T o   s t o r e   l a s t   c h o s e n   r e n d e r e r   t y p e   
  
 j     �� �� &0 previousselection previousSelection  m     ����       l     ��������  ��  ��     ��  i        I     �� ��
�� .aevtoappnull  �   � ****  o      ���� 0 argv  ��    k    ;       l     ��  ��    / ) Get absolute path to SSR binary via args     �   R   G e t   a b s o l u t e   p a t h   t o   S S R   b i n a r y   v i a   a r g s      l     ��  ��    \ V For relative paths to work, the binary must be placed in MacOS/ folder of .app bundle     �   �   F o r   r e l a t i v e   p a t h s   t o   w o r k ,   t h e   b i n a r y   m u s t   b e   p l a c e d   i n   M a c O S /   f o l d e r   o f   . a p p   b u n d l e      r        !   n      " # " 4   �� $
�� 
cobj $ m    ����  # o     ���� 0 argv   ! o      ���� 0 
ssrbinpath 
ssrBinPath   % & % r     ' ( ' n    
 ) * ) 1    
��
�� 
rest * o    ���� 0 argv   ( o      ���� 0 argv   &  + , + l   ��������  ��  ��   ,  - . - l   �� / 0��   / G A Make list of renderer type names and their parameter equivalents    0 � 1 1 �   M a k e   l i s t   o f   r e n d e r e r   t y p e   n a m e s   a n d   t h e i r   p a r a m e t e r   e q u i v a l e n t s .  2 3 2 r     4 5 4 J     6 6  7 8 7 m     9 9 � : : , B i n a u r a l   ( u s i n g   H R I R s ) 8  ; < ; m     = = � > > J B i n a u r a l   R o o m   S y n t h e s i s   ( u s i n g   B R I R s ) <  ? @ ? m     A A � B B 4 B i n a u r a l   P l a y b a c k   R e n d e r e r @  C D C m     E E � F F ( W a v e   F i e l d   S y n t h e s i s D  G H G m     I I � J J 8 A m b i s o n i c s   A m p l i t u d e   P a n n i n g H  K L K m     M M � N N X S t e r e o p h o n i c   ( V e c t o r   B a s e   A m p l i t u d e   P a n n i n g ) L  O�� O m     P P � Q Q   G e n e r i c   R e n d e r e r��   5 o      ���� 0 renderernames rendererNames 3  R S R r    , T U T J    ( V V  W X W m     Y Y � Z Z  - - b i n a u r a l X  [ \ [ m     ] ] � ^ ^ 
 - - b r s \  _ ` _ m     a a � b b 
 - - b p b `  c d c m     e e � f f 
 - - w f s d  g h g m      i i � j j 
 - - a a p h  k l k m     # m m � n n  - - v b a p l  o�� o m   # & p p � q q  - - g e n e r i c��   U o      ���� "0 rendereroptions rendererOptions S  r s r l  - -��������  ��  ��   s  t u t l  - -�� v w��   v / ) Process other given command line options    w � x x R   P r o c e s s   o t h e r   g i v e n   c o m m a n d   l i n e   o p t i o n s u  y z y r   - 2 { | { m   - .��
�� boovtrue | o      ���� $0 pickrenderertype pickRendererType z  } ~ } r   3 >  �  c   3 : � � � m   3 6 � � � � �   � m   6 9��
�� 
utxt � o      ���� 0 options   ~  � � � X   ? u ��� � � k   Q p � �  � � � r   Q ^ � � � b   Q Z � � � b   Q X � � � o   Q T���� 0 options   � m   T W � � � � �    � o   X Y���� 0 arg   � o      ���� 0 options   �  ��� � Z   _ p � ����� � E  _ d � � � o   _ b���� "0 rendereroptions rendererOptions � o   b c���� 0 arg   � r   g l � � � m   g h��
�� boovfals � o      ���� $0 pickrenderertype pickRendererType��  ��  ��  �� 0 arg   � o   B C���� 0 argv   �  � � � l  v v��������  ��  ��   �  � � � l  v v�� � ���   � < 6 Let user pick renderer type if none was given in argv    � � � � l   L e t   u s e r   p i c k   r e n d e r e r   t y p e   i f   n o n e   w a s   g i v e n   i n   a r g v �  � � � r   v � � � � c   v } � � � m   v y � � � � �   � m   y |��
�� 
utxt � o      ����  0 rendereroption rendererOption �  � � � Z   �( � ����� � l  � � ����� � o   � ����� $0 pickrenderertype pickRendererType��  ��   � k   �$ � �  � � � Z   � � � ����� � ?  � � � � � o   � ����� &0 previousselection previousSelection � l  � � ����� � I  � ��� ���
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
cobj � o   � ����� &0 previousselection previousSelection � o   � ����� 0 renderernames rendererNames��  ��   � o      ���� ,0 selectedrenderername selectedRendererName��   � m   � � � ��                                                                                  sevs  alis    �  Macintosh HD               �i��H+   K�System Events.app                                               v�8CW        ����  	                CoreServices    �is�      �8'7     K� K\ K[  :Macintosh HD:System:Library:CoreServices:System Events.app  $  S y s t e m   E v e n t s . a p p    M a c i n t o s h   H D  -System/Library/CoreServices/System Events.app   / ��   �  ��� � Z   �$ � ��� � � =  � � � � � o   � ����� ,0 selectedrenderername selectedRendererName � m   � ���
�� boovfals � L   � �����  ��   � k   �$ � �  � � � r   � � � � � n   � � � � � 4  � ��� �
�� 
cobj � m   � �����  � o   � ����� ,0 selectedrenderername selectedRendererName � o      ���� ,0 selectedrenderername selectedRendererName �  ��� � Y   �$ ��� � ��� � Z   � � ����� � =  � � � � o   � ����� ,0 selectedrenderername selectedRendererName � l  � ����� � n   � � � � 4   �� �
�� 
cobj � o  ���� 0 i   � o   � ���� 0 renderernames rendererNames��  ��   � k   � �  � � � r   � � � n   � � � 4  
�� �
�� 
cobj � o  ���� 0 i   � o  
���� "0 rendereroptions rendererOptions � o      ����  0 rendereroption rendererOption �  � � � l �� � ���   �   Save user choice     � � � � $   S a v e   u s e r   c h o i c e   �  � � � r   � � � o  ���� 0 i   � o      ���� &0 previousselection previousSelection �  ��   S  ��  ��  ��  �� 0 i   � m   � �����  � l  � ����� I  � �����
�� .corecnte****       **** n  � � 2  � ���
�� 
cobj o   � ����� 0 renderernames rendererNames��  ��  ��  ��  ��  ��  ��  ��   �  l ))��������  ��  ��    l ))��	
��  	 $  Assemble shell command string   
 � <   A s s e m b l e   s h e l l   c o m m a n d   s t r i n g  r  )N b  )J b  )F b  )B b  )> b  ): b  )8 b  )4 b  )2 b  ). !  m  ),"" �## " e x p o r t   E C A S O U N D = "! o  ,-���� 0 
ssrbinpath 
ssrBinPath m  .1$$ �%% " / e c a s o u n d "   ;   c d   " o  23���� 0 
ssrbinpath 
ssrBinPath m  47&& �''  / . . / . . / . . "   ;   " o  89���� 0 
ssrbinpath 
ssrBinPath m  :=(( �))  / s s r "   o  >A����  0 rendereroption rendererOption o  BE���� 0 options   m  FI** �++ �   & &   ( e c h o   S S R   q u i t   n o r m a l l y   w i t h   e x i t   c o d e   $ ? )   | |   ( e c h o   S S R   e n c o u n t e r e d   a n   e r r o r   a n d   h a d   t o   q u i t .   S e e   a b o v e .     E x i t   c o d e :   $ ? ) o      �� 0 command   ,-, l OO�~�}�|�~  �}  �|  - ./. l OO�{01�{  0 D > Open new Terminal window, cd to SSR working dir and start SSR   1 �22 |   O p e n   n e w   T e r m i n a l   w i n d o w ,   c d   t o   S S R   w o r k i n g   d i r   a n d   s t a r t   S S R/ 343 O Of565 r  Ue787 l Ua9�z�y9 I Ua�x:�w
�x .coredoexbool       obj : 4  U]�v;
�v 
prcs; m  Y\<< �==  T e r m i n a l�w  �z  �y  8 o      �u�u "0 terminalrunning terminalRunning6 m  OR>>�                                                                                  sevs  alis    �  Macintosh HD               �i��H+   K�System Events.app                                               v�8CW        ����  	                CoreServices    �is�      �8'7     K� K\ K[  :Macintosh HD:System:Library:CoreServices:System Events.app  $  S y s t e m   E v e n t s . a p p    M a c i n t o s h   H D  -System/Library/CoreServices/System Events.app   / ��  4 ?@? O  g9ABA k  m8CC DED Z  m�FG�tHF o  mp�s�s "0 terminalrunning terminalRunningG k  s�II JKJ I sx�r�q�p
�r .miscactvnull��� ��� null�q  �p  K L�oL r  y�MNM I y��nO�m
�n .coredoscnull��� ��� ctxtO o  y|�l�l 0 command  �m  N o      �k�k 0 ssrtab ssrTab�o  �t  H l ��PQRP k  ��SS TUT I ���j�i�h
�j .miscactvnull��� ��� null�i  �h  U V�gV r  ��WXW I ���fYZ
�f .coredoscnull��� ��� ctxtY o  ���e�e 0 command  Z �d[�c
�d 
kfil[ 4 ���b\
�b 
cwin\ m  ���a�a �c  X o      �`�` 0 ssrtab ssrTab�g  Q F @ Terminal not running yet. Start it and use fresh default window   R �]] �   T e r m i n a l   n o t   r u n n i n g   y e t .   S t a r t   i t   a n d   u s e   f r e s h   d e f a u l t   w i n d o wE ^_^ l ���_�^�]�_  �^  �]  _ `a` l ���\bc�\  b 4 . Position Terminal window in upper left corner   c �dd \   P o s i t i o n   T e r m i n a l   w i n d o w   i n   u p p e r   l e f t   c o r n e ra efe r  ��ghg J  ��ii jkj m  ���[�[  k l�Zl m  ���Y�Y  �Z  h n      mnm 1  ���X
�X 
pposn 4 ���Wo
�W 
cwino m  ���V�V f pqp l ���U�T�S�U  �T  �S  q rsr l ���Rtu�R  t   Wait until SSR quits   u �vv *   W a i t   u n t i l   S S R   q u i t ss wxw I ���Qy�P
�Q .sysodelanull��� ��� nmbry m  ���O�O �P  x z{z W  ��|}| I ���N~�M
�N .sysodelanull��� ��� nmbr~ m  ���L�L �M  } l ���K�J E  ����� n ����� 1  ���I
�I 
hist� o  ���H�H 0 ssrtab ssrTab� o  ���G�G 0 command  �K  �J  { ��� V  ����� I ���F��E
�F .sysodelanull��� ��� nmbr� m  ���D�D �E  � l ����C�B� = ����� n  ����� 1  ���A
�A 
busy� o  ���@�@ 0 ssrtab ssrTab� m  ���?
�? boovtrue�C  �B  � ��� l ���>�=�<�>  �=  �<  � ��� l ���;���;  � ? 9 Close Terminal window if SSR did quit with exit code 0		   � ��� r   C l o s e   T e r m i n a l   w i n d o w   i f   S S R   d i d   q u i t   w i t h   e x i t   c o d e   0 	 	� ��:� P  �8��9�� Z  �7���8�7� F  ���� l ����6�5� E  ����� n ����� 1  ���4
�4 
hist� o  ���3�3 0 ssrtab ssrTab� m  ���� ��� D S S R   q u i t   n o r m a l l y   w i t h   e x i t   c o d e   0�6  �5  � l ��2�1� H  �� E  ��� n ��� 1  �0
�0 
hist� o  �/�/ 0 ssrtab ssrTab� m  �� ���  - - h e l p�2  �1  � I 3�.��
�. .coreclosnull���     obj � l )��-�,� 6 )��� 4 �+�
�+ 
cwin� m  �*�* � = (��� 1  "�)
�) 
tcnt� o  #'�(�( 0 ssrtab ssrTab�-  �,  � �'��&
�' 
savo� m  ,/�%
�% savono  �&  �8  �7  �9  � �$�#
�$ conscase�#  �:  B m  gj���                                                                                      @ alis    j  Macintosh HD               �i��H+   K�Terminal.app                                                    �n�wc�        ����  	                	Utilities     �is�      �wG�     K� K�  0Macintosh HD:Applications:Utilities:Terminal.app    T e r m i n a l . a p p    M a c i n t o s h   H D  #Applications/Utilities/Terminal.app   / ��  @ ��"� l ::�!� ��!  �   �  �"  ��       �����  � ��� &0 previousselection previousSelection
� .aevtoappnull  �   � ****� � � �����
� .aevtoappnull  �   � ****� 0 argv  �  � ���� 0 argv  � 0 arg  � 0 i  � C��� 9 = A E I M P�� Y ] a e i m p�� ����
�	 � �� ��� �� �����"$&(*� ��<�����������������������������������
� 
cobj� 0 
ssrbinpath 
ssrBinPath
� 
rest� � 0 renderernames rendererNames� "0 rendereroptions rendererOptions� $0 pickrenderertype pickRendererType
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
inSL� 
� .gtqpchltns    @   @ ns  � ,0 selectedrenderername selectedRendererName�  0 command  
�� 
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
�� .coreclosnull���     obj �<��k/E�O��,E�O��������vE�O����a a a �vE` OeE` Oa a &E` O 5�[a �l kh _ a %�%E` O_ � 
fE` Y h[OY��Oa a &E` O_  �b   �j  kEc   Y hOa  ,*j O�a a  a !a "a #��b   /kva $ %E` &UO_ &f  hY E_ &�k/E` &O 7k��-j kh _ &��/  _ �/E` O�Ec   OY h[OY��Y hOa '�%a (%�%a )%�%a *%_ %_ %a +%E` ,Oa  *a -a ./j /E` 0UOa 1 �_ 0 *j O_ ,j 2E` 3Y *j O_ ,a 4*a 5k/l 2E` 3Ojjlv*a 5k/a 6,FOlj 7O h_ 3a 8,_ ,kj 7[OY��O h_ 3a 9,e kj 7[OY��Oga : G_ 3a 8,a ;	 _ 3a 8,a <a =& $*a 5k/a >[a ?,\Z_ 381a @a Al BY hVUOPascr  ��ޭ