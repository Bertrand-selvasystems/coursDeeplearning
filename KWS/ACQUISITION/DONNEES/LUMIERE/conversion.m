% D�finir la plage des fichiers � traiter
startIndex = 12;
endIndex = 129;

% Boucle sur chaque fichier dans la plage
for i = startIndex:endIndex
    % Cr�er le nom de fichier en fonction de l'index
    inputFileName = sprintf('spectro_%d.txt', i);
    outputFileName = sprintf('spectro_%d.csv', i);
    
    % V�rifier si le fichier existe
    if exist(inputFileName, 'file') == 2
        % Lire les donn�es avec un s�parateur d'espace
        data = importdata(inputFileName, ' ');
        
        % V�rification des dimensions de la matrice
        [rows, cols] = size(data);
        fprintf('Le fichier %s contient une matrice de taille %d x %d.\n', inputFileName, rows, cols);
        
        % �crire les donn�es dans un fichier CSV
        dlmwrite(outputFileName, data, 'delimiter', ',');
        
        % Message de confirmation
        fprintf('Fichier %s converti en %s\n', inputFileName, outputFileName);
    else
        fprintf('Fichier %s non trouv�.\n', inputFileName);
    end
end

disp('Conversion termin�e.');
